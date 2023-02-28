#include "mgmpch.h"
#include "VulkanDevice.h"

#include "VulkanInstance.h"
#include "VulkanSurface.h"

namespace Magma
{
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

	static bool IsDeviceSuitable(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface,
		const PhysicalDeviceRequirements& requirements, VulkanDeviceQueueIndices& outQueueIndices)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

		if (physicalDeviceProperties.deviceType != GetDeviceType(requirements.DeviceType))
			return false;

		VulkanDeviceQueueIndices queueIndices = FindQueueIndices(physicalDevice, surface);
		if (!queueIndices.IsValid())
			return false;

		outQueueIndices = queueIndices;
		return true;
	}

	VulkanDevice::VulkanDevice(GraphicsInstance& instance, const Scope<RenderSurface>& surface, const PhysicalDeviceRequirements& requirements)
	{
		VkInstance instanceHandle = dynamic_cast<VulkanInstance*>(&instance)->GetInstanceHandle();
		MGM_CORE_ASSERT(instanceHandle, "Invalid Graphics Instance!");
		VkSurfaceKHR surfaceHandle = dynamic_cast<VulkanSurface*>(surface.get())->GetHandle();
		MGM_CORE_ASSERT(surfaceHandle, "Invalid Render Surfce!");
		PickPhysicalDevice(instanceHandle, surfaceHandle, requirements);
	}

	VulkanDevice::VulkanDevice(const Scope<GraphicsInstance>& instance, const Scope<RenderSurface>& surface, const PhysicalDeviceRequirements& requirements)
	{
		VkInstance instanceHandle = dynamic_cast<VulkanInstance*>(instance.get())->GetInstanceHandle();
		MGM_CORE_ASSERT(instanceHandle, "Invalid Graphics Instance!");
		VkSurfaceKHR surfaceHandle = dynamic_cast<VulkanSurface*>(surface.get())->GetHandle();
		MGM_CORE_ASSERT(surfaceHandle, "Invalid Render Surfce!");
		PickPhysicalDevice(instanceHandle, surfaceHandle, requirements);
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
}