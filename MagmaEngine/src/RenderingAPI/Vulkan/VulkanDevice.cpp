#include "mgmpch.h"
#include "VulkanDevice.h"

#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "ValidationLayers.h"

namespace Magma
{
	using DeviceSuitability = VulkanPhysicalDeviceSelectionOptions::Suitability;

	static const std::vector<const char*> c_DeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	static VulkanDeviceQueueIndices FindQueueIndices(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface)
	{
		VulkanDeviceQueueIndices indices;

		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		u32 i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.GraphicsQueue = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport)
				indices.PresentQueue = i;

			if (indices.IsValid()) break;

			i++;
		}

		return indices;
	}

	static VkPhysicalDeviceType GetVkPhysicalDeviceType(PhysicalDeviceType deviceType)
	{
		switch (deviceType)
		{
			case PhysicalDeviceType::Other:         return VK_PHYSICAL_DEVICE_TYPE_OTHER;
			case PhysicalDeviceType::IntegratedGPU: return VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
			case PhysicalDeviceType::DedicatedGPU:  return VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			case PhysicalDeviceType::VirtualGPU:    return VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
			case PhysicalDeviceType::CPU:           return VK_PHYSICAL_DEVICE_TYPE_CPU;
			default:
				return VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM;
		}
	}

	static PhysicalDeviceType GetDeviceType(VkPhysicalDeviceType deviceType)
	{
		switch (deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:          return PhysicalDeviceType::Other;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return PhysicalDeviceType::IntegratedGPU;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:	 return PhysicalDeviceType::DedicatedGPU;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:	 return PhysicalDeviceType::VirtualGPU;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:			 return PhysicalDeviceType::CPU;
			default:
				return PhysicalDeviceType::Unknown;
		}
	}

	static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const PhysicalDeviceRequirements& requirements)
	{
		u32 extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::unordered_set<std::string> requiredExtensions(c_DeviceExtensions.begin(), c_DeviceExtensions.end());

		if (requirements.SupportsRaytracing)
		{
			requiredExtensions.emplace(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
			requiredExtensions.emplace(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
			requiredExtensions.emplace(VK_KHR_RAY_QUERY_EXTENSION_NAME);
			requiredExtensions.emplace(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
			requiredExtensions.emplace(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		}

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	static void PopulatePhysicalDeviceOptions(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, VulkanPhysicalDeviceSelectionOptions& outPhysicalDeviceOptions)
	{
		outPhysicalDeviceOptions.PhysicalDevice = physicalDevice;
		vkGetPhysicalDeviceProperties(physicalDevice, &outPhysicalDeviceOptions.Properties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &outPhysicalDeviceOptions.Features);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &outPhysicalDeviceOptions.MemoryProperties);

		outPhysicalDeviceOptions.Type = GetDeviceType(outPhysicalDeviceOptions.Properties.deviceType);
		outPhysicalDeviceOptions.Surface = surface;
	}

	static void CalculateDeviceScore(VulkanPhysicalDeviceSelectionOptions& physicalDeviceOptions)
	{
		// We prefer full suitability even if 
		if (physicalDeviceOptions.Suitable == DeviceSuitability::Yes)
			physicalDeviceOptions.DeviceScore += 1000;
		else if (physicalDeviceOptions.Suitable == DeviceSuitability::Partial)
			physicalDeviceOptions.DeviceScore += 500;

		const VkPhysicalDeviceLimits& limits = physicalDeviceOptions.Properties.limits;
		VkSampleCountFlags counts = limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts;

		// The more samples it can support, the more points we add to the device
		for (VkSampleCountFlagBits bit = VK_SAMPLE_COUNT_64_BIT; bit; bit = (VkSampleCountFlagBits)(bit >> 1))
		{
			if (counts & bit)
			{
				physicalDeviceOptions.DeviceScore += (bit * 100);
				break;
			}
		}

		// The VRAM size can be determined by finding the memory type with the exclusive DEVICE_LOCAL bit
		// and then querying the size of the memory heap it belongs to
		u32 memoryCount = physicalDeviceOptions.MemoryProperties.memoryTypeCount;
		std::vector<VkMemoryType> memoryTypes(memoryCount);

		for (int i = 0; i < memoryCount; i++)
			memoryTypes[i] = physicalDeviceOptions.MemoryProperties.memoryTypes[i];

		std::stable_sort(memoryTypes.begin(), memoryTypes.end(), [](const auto& lhs, const auto& rhs) { return lhs.heapIndex < rhs.heapIndex; });
		i32 heapIndex = -1;

		for (const VkMemoryType& type : memoryTypes)
		{
			// We check if this memory type only has the DEVICE_LOCAL bit
			bool deviceLocal = (type.propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			// And if we have already counted this heap
			bool foundHeap = (heapIndex < (i32)type.heapIndex);
			if (deviceLocal && foundHeap)
			{
				heapIndex = type.heapIndex;
				VkMemoryHeap deviceLocalHeap = physicalDeviceOptions.MemoryProperties.memoryHeaps[heapIndex];

				physicalDeviceOptions.DeviceScore += (deviceLocalHeap.size >> 20);
			}
		}

		// TODO: consider more device options
	}

	static DeviceSuitability SelectDeviceSuitability(const VulkanPhysicalDeviceSelectionOptions& physicalDeviceOptions,
		const PhysicalDeviceRequirements& requirements, VulkanDeviceQueueIndices& outQueueIndices)
	{
		DeviceSuitability suitability = DeviceSuitability::Yes;

		VkPhysicalDevice physicalDevice = physicalDeviceOptions.PhysicalDevice;
		VkPhysicalDeviceProperties physicalDeviceProperties = physicalDeviceOptions.Properties;
		VkPhysicalDeviceFeatures physicalDeviceFeatures = physicalDeviceOptions.Features;
		VkSurfaceKHR surface = physicalDeviceOptions.Surface;

		if (physicalDeviceProperties.deviceType != GetVkPhysicalDeviceType(requirements.DeviceType))
			suitability = DeviceSuitability::Partial;

		if (requirements.SupportsGeometryShaders && !physicalDeviceFeatures.geometryShader)
			return DeviceSuitability::No;

		if (requirements.SupportsTesselationShaders && !physicalDeviceFeatures.tessellationShader)
			return DeviceSuitability::No;

		if (requirements.SupportsAnisotropy && !physicalDeviceFeatures.samplerAnisotropy)
			return DeviceSuitability::No;

		VulkanDeviceQueueIndices queueIndices = FindQueueIndices(physicalDevice, surface);
		if (!queueIndices.IsValid())
			return DeviceSuitability::No;

		if (!CheckDeviceExtensionSupport(physicalDevice, requirements))
			return DeviceSuitability::No;

		// TODO: This CAN change during execution, handle it if possible
		if (const auto& swapchainSupport = VulkanSwapchain::QuerySwapchainSupportDetails(physicalDevice, surface);
			swapchainSupport.Formats.empty() || swapchainSupport.PresentModes.empty())
			return DeviceSuitability::No;

		outQueueIndices = queueIndices;
		return suitability;
	}

	static bool IsDeviceSuitable(const PhysicalDeviceRequirements& requirements,
		VulkanPhysicalDeviceSelectionOptions& physicalDeviceOptions)
	{
		physicalDeviceOptions.Suitable = SelectDeviceSuitability(physicalDeviceOptions, requirements, physicalDeviceOptions.QueueIndices);
		return physicalDeviceOptions.Suitable != DeviceSuitability::No;
	}

	VulkanDevice::VulkanDevice(const RawPointer<GraphicsInstance>& instance, const RawPointer<RenderSurface>& surface, const PhysicalDeviceRequirements& requirements)
	{
		VkInstance instanceHandle = dynamic_cast<VulkanInstance*>(instance.Get())->GetInstanceHandle();
		MGM_CORE_ASSERT(instanceHandle, "Invalid Graphics Instance!");
		VkSurfaceKHR surfaceHandle = dynamic_cast<VulkanSurface*>(surface.Get())->GetHandle();
		MGM_CORE_ASSERT(surfaceHandle, "Invalid Render Surfce!");
		PickPhysicalDevice(instanceHandle, surfaceHandle, requirements);
		CreateLogicalDevice(requirements.SupportsRaytracing);

		MGM_CORE_INFO("Successfully created Render Device!");
	}

	VulkanDevice::~VulkanDevice()
	{
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

	void VulkanDevice::PickPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface, const PhysicalDeviceRequirements& requirements)
	{
		u32 physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		MGM_CORE_ASSERT(physicalDeviceCount > 0, "Unable to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> availablePhysicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, availablePhysicalDevices.data());

		std::vector<VulkanPhysicalDeviceSelectionOptions> suitablePhysicalDevices;
		suitablePhysicalDevices.reserve(physicalDeviceCount);

		for (const auto& physicalDevice : availablePhysicalDevices)
		{
			VulkanPhysicalDeviceSelectionOptions physicalDeviceOptions = {};
			PopulatePhysicalDeviceOptions(physicalDevice, surface, physicalDeviceOptions);
			if (IsDeviceSuitable(requirements, physicalDeviceOptions))
			{
				CalculateDeviceScore(physicalDeviceOptions);
				suitablePhysicalDevices.push_back(physicalDeviceOptions);
			}
		}

		if (suitablePhysicalDevices.size())
		{
			// TODO: include in selection policy
			if (bool allowPartialSuitability = true)
				std::stable_sort(suitablePhysicalDevices.begin(), suitablePhysicalDevices.end(), [](const auto& lhs, const auto& rhs) { return lhs.DeviceScore > rhs.DeviceScore; });
			else
				std::stable_partition(suitablePhysicalDevices.begin(), suitablePhysicalDevices.end(), [](const auto& pd) { return pd.Suitable == DeviceSuitability::Yes; });

			VulkanPhysicalDeviceSelectionOptions selectedPhysicalDevice = suitablePhysicalDevices[0];

			m_PhysicalDevice = selectedPhysicalDevice.PhysicalDevice;
			m_QueueIndices = selectedPhysicalDevice.QueueIndices;
			m_DeviceProperties = selectedPhysicalDevice.Properties;
			m_DeviceFeatures = selectedPhysicalDevice.Features;
			m_DeviceMemoryProperties = selectedPhysicalDevice.MemoryProperties;

			// TODO: Remove when selection device system is done
			MGM_CORE_TRACE("Physical Device Score: {}", selectedPhysicalDevice.DeviceScore);
		}

		MGM_CORE_ASSERT(m_PhysicalDevice, "Failed to find a suitable GPU!");
		MGM_CORE_VERIFY(m_QueueIndices.IsValid());
	}

	void VulkanDevice::CreateLogicalDevice(bool raytracingSupport)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::unordered_set<u32> uniqueQueueFamilies = { m_QueueIndices.GraphicsQueue, m_QueueIndices.PresentQueue };

		float queuePriority = 1.0f;
		for (const auto& queueIndex : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createInfo.queueFamilyIndex = queueIndex;
			createInfo.queueCount = 1;
			createInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(createInfo);
		}

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &m_DeviceFeatures;

		if (ValidationLayers::Enabled())
		{
			auto& layers = ValidationLayers::GetLayers();
			deviceCreateInfo.enabledLayerCount = static_cast<u32>(layers.size());
			deviceCreateInfo.ppEnabledLayerNames = layers.data();
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
			deviceCreateInfo.ppEnabledLayerNames = nullptr;
		}

		deviceCreateInfo.enabledExtensionCount = static_cast<u32>(c_DeviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = c_DeviceExtensions.data();

		if (raytracingSupport)
		{
			VkPhysicalDeviceVulkan12Features vulkan12Features = {};
			vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			vulkan12Features.descriptorIndexing = VK_TRUE;
			vulkan12Features.bufferDeviceAddress = VK_TRUE;

			VkPhysicalDeviceAccelerationStructureFeaturesKHR accStructFeatures = {};
			accStructFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			accStructFeatures.accelerationStructure = VK_TRUE;
			accStructFeatures.pNext = &vulkan12Features;

			VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures = {};
			rtPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			rtPipelineFeatures.rayTracingPipeline = VK_TRUE;
			rtPipelineFeatures.pNext = &accStructFeatures;

			deviceCreateInfo.pNext = &rtPipelineFeatures;
		}

		VkResult result = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_LogicalDevice);
		MGM_CORE_VERIFY(result == VK_SUCCESS);

		vkGetDeviceQueue(m_LogicalDevice, m_QueueIndices.GraphicsQueue, 0, &m_QueueHandles.GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_QueueIndices.PresentQueue, 0, &m_QueueHandles.PresentQueue);
	}
}