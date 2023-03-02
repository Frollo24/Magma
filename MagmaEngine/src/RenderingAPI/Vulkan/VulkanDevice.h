#pragma once
#include "Magma/Renderer/RenderDevice.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	struct VulkanDeviceQueueIndices
	{
		u32 GraphicsQueue = VK_QUEUE_FAMILY_IGNORED;
		u32 PresentQueue = VK_QUEUE_FAMILY_IGNORED;

		bool IsValid() const
		{
			return GraphicsQueue != VK_QUEUE_FAMILY_IGNORED && PresentQueue != VK_QUEUE_FAMILY_IGNORED;
		}
	};

	struct VulkanDeviceQueueHandles
	{
		VkQueue GraphicsQueue = VK_NULL_HANDLE;
		VkQueue PresentQueue = VK_NULL_HANDLE;
	};

	class VulkanDevice : public RenderDevice
	{
	public:
		VulkanDevice(GraphicsInstance& instance, RenderSurface& surface, const PhysicalDeviceRequirements& requirements);
		VulkanDevice(const Scope<GraphicsInstance>& instance, const Scope<RenderSurface>& surface, const PhysicalDeviceRequirements& requirements);
		virtual ~VulkanDevice();

		inline VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		inline VkDevice GetLogicalDevice() const { return m_LogicalDevice; }

		inline const VulkanDeviceQueueIndices& GetQueueIndices() const { return m_QueueIndices; }

	private:
		void PickPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface, const PhysicalDeviceRequirements& requirements);
		void CreateLogicalDevice();

	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;

		VulkanDeviceQueueIndices m_QueueIndices;
		VulkanDeviceQueueHandles m_QueueHandles;
	};
}

