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

	struct VulkanPhysicalDeviceSelectionOptions
	{
		u32 DeviceScore = 0;
		enum class Suitability { No, Partial, Yes };
		Suitability Suitable = Suitability::No;

		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties Properties = { 0 };
		VkPhysicalDeviceMemoryProperties MemoryProperties = { 0 };
		VkPhysicalDeviceFeatures Features = { 0 };
		PhysicalDeviceType Type = PhysicalDeviceType::Other;

		VkSurfaceKHR Surface = VK_NULL_HANDLE;

		VulkanDeviceQueueIndices QueueIndices = {};
	};

	class VulkanDevice : public RenderDevice
	{
	public:
		VulkanDevice(const RawPointer<GraphicsInstance>& instance, const RawPointer<RenderSurface>& surface, const PhysicalDeviceRequirements& requirements);
		virtual ~VulkanDevice();

		inline VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		inline VkDevice GetLogicalDevice() const { return m_LogicalDevice; }

		inline const VkPhysicalDeviceProperties& GetProperties() const { return m_DeviceProperties; }
		inline const VkPhysicalDeviceFeatures& GetFeatures() const { return m_DeviceFeatures; }
		inline const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_DeviceMemoryProperties; }

		inline const VulkanDeviceQueueIndices& GetQueueIndices() const { return m_QueueIndices; }
		inline const VulkanDeviceQueueHandles& GetQueueHandles() const { return m_QueueHandles; }

	private:
		void PickPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface, const PhysicalDeviceRequirements& requirements);
		void CreateLogicalDevice(bool raytracingSupport);

	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;

		VkPhysicalDeviceProperties m_DeviceProperties = { 0 };
		VkPhysicalDeviceFeatures m_DeviceFeatures = { 0 };
		VkPhysicalDeviceMemoryProperties m_DeviceMemoryProperties = { 0 };

		VulkanDeviceQueueIndices m_QueueIndices;
		VulkanDeviceQueueHandles m_QueueHandles;
	};
}

