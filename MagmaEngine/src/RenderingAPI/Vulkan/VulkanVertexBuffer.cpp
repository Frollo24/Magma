#include "mgmpch.h"
#include "VulkanVertexBuffer.h"

#include "RenderingAPI/Vulkan/VulkanDevice.h"
#include "RenderingAPI/Vulkan/VulkanContext.h"
#include "VulkanUtils.h"

namespace Magma
{
	VulkanVertexBuffer::VulkanVertexBuffer(const Ref<RenderDevice>& device, u32 size, const void* data)
	{
		Ref<VulkanDevice> vulkanDevice = DynamicCast<VulkanDevice>(device);
		VkPhysicalDevice physicalDevice = vulkanDevice->GetPhysicalDevice();
		m_Device = vulkanDevice->GetLogicalDevice();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VkDeviceSize memorySize = size;
		CreateBuffer(physicalDevice, m_Device, memorySize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* bufferData;
		vkMapMemory(m_Device, stagingBufferMemory, 0, memorySize, 0, &bufferData);
		memcpy(bufferData, data, memorySize);
		vkUnmapMemory(m_Device, stagingBufferMemory);

		CreateBuffer(physicalDevice, m_Device, memorySize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_BufferMemory);

		VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(RenderContext::Instance().get());
		VkCommandPool commandPool = vulkanContext->GetCommandPool();

		VkCommandBuffer copyCommand = BeginSingleTimeCommands(commandPool, m_Device);
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = memorySize;
		vkCmdCopyBuffer(copyCommand, stagingBuffer, m_Buffer, 1, &copyRegion);
		EndSingleTimeCommands(commandPool, m_Device, copyCommand, vulkanDevice->GetQueueHandles().GraphicsQueue);

		vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		vkDestroyBuffer(m_Device, m_Buffer, nullptr);
		vkFreeMemory(m_Device, m_BufferMemory, nullptr);
	}
}