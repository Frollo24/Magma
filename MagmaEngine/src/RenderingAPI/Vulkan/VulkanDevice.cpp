#include "mgmpch.h"
#include "VulkanDevice.h"

#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "ValidationLayers.h"

namespace Magma
{
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

	static VkPhysicalDeviceType GetDeviceType(PhysicalDeviceType deviceType)
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

	static bool IsDeviceSuitable(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface,
		const PhysicalDeviceRequirements& requirements, VulkanDeviceQueueIndices& outQueueIndices)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

		if (physicalDeviceProperties.deviceType != GetDeviceType(requirements.DeviceType))
			return false;

		if (requirements.SupportsGeometryShaders && !physicalDeviceFeatures.geometryShader)
			return false;

		if (requirements.SupportsTesselationShaders && !physicalDeviceFeatures.tessellationShader)
			return false;

		if (requirements.SupportsAnisotropy && !physicalDeviceFeatures.samplerAnisotropy)
			return false;

		VulkanDeviceQueueIndices queueIndices = FindQueueIndices(physicalDevice, surface);
		if (!queueIndices.IsValid())
			return false;

		if (!CheckDeviceExtensionSupport(physicalDevice, requirements))
			return false;

		if (const auto& swapchainSupport = VulkanSwapchain::QuerySwapchainSupportDetails(physicalDevice, surface);
			swapchainSupport.Formats.empty() || swapchainSupport.PresentModes.empty())
			return false;

		outQueueIndices = queueIndices;
		return true;
	}

	VulkanDevice::VulkanDevice(const RawPointer<GraphicsInstance>& instance, const RawPointer<RenderSurface>& surface, const PhysicalDeviceRequirements& requirements)
	{
		VkInstance instanceHandle = dynamic_cast<VulkanInstance*>(instance.Get())->GetInstanceHandle();
		MGM_CORE_ASSERT(instanceHandle, "Invalid Graphics Instance!");
		VkSurfaceKHR surfaceHandle = dynamic_cast<VulkanSurface*>(surface.Get())->GetHandle();
		MGM_CORE_ASSERT(surfaceHandle, "Invalid Render Surfce!");
		PickPhysicalDevice(instanceHandle, surfaceHandle, requirements);
		CreateLogicalDevice();

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

		VulkanDeviceQueueIndices queueIndices;
		for (const auto& physicalDevice : availablePhysicalDevices)
		{
			if (IsDeviceSuitable(physicalDevice, surface, requirements, queueIndices))
			{
				m_PhysicalDevice = physicalDevice;
				m_QueueIndices = queueIndices;
				break;
			}
		}

		MGM_CORE_ASSERT(m_PhysicalDevice, "Failed to find a suitable GPU!");
		MGM_CORE_VERIFY(m_QueueIndices.IsValid());
	}

	void VulkanDevice::CreateLogicalDevice()
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

		VkPhysicalDeviceFeatures physicalDeviceFeatures{};
		physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

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

		VkResult result = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_LogicalDevice);
		MGM_CORE_VERIFY(result == VK_SUCCESS);

		vkGetDeviceQueue(m_LogicalDevice, m_QueueIndices.GraphicsQueue, 0, &m_QueueHandles.GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_QueueIndices.PresentQueue, 0, &m_QueueHandles.PresentQueue);
	}
}