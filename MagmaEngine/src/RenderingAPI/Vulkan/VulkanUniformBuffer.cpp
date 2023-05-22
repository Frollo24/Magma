#include "mgmpch.h"
#include "VulkanUniformBuffer.h"

#include "RenderingAPI/Vulkan/VulkanDevice.h"
#include "RenderingAPI/Vulkan/VulkanContext.h"
#include "VulkanUtils.h"

namespace Magma
{
	VulkanUniformBuffer::VulkanUniformBuffer(const Ref<RenderDevice>& device, u32 size, u32 binding, u32 maxFrames)
		: UniformBuffer(binding, maxFrames), m_Device(DynamicCast<VulkanDevice>(device)->GetLogicalDevice())
	{
		Ref<VulkanDevice> vulkanDevice = DynamicCast<VulkanDevice>(device);
		VkPhysicalDevice physicalDevice = vulkanDevice->GetPhysicalDevice();

		m_Buffers.resize(maxFrames);
		m_BufferMemories.resize(maxFrames);
		m_BufferMappings.resize(maxFrames);

		for (size_t i = 0; i < maxFrames; i++) {
			CreateBuffer(physicalDevice, m_Device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Buffers[i], m_BufferMemories[i]);

			vkMapMemory(m_Device, m_BufferMemories[i], 0, size, 0, &m_BufferMappings[i]);
		}
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		for (const auto& buffer : m_Buffers)
			vkDestroyBuffer(m_Device, buffer, nullptr);

		for (const auto& memory : m_BufferMemories)
			vkFreeMemory(m_Device, memory, nullptr);

		for (auto& mapping : m_BufferMappings)
			mapping = nullptr;

		m_BufferMappings.clear();
	}

	void VulkanUniformBuffer::SetCommonDataForAllFrames(const void* data, u32 size, u32 offset)
	{
		for (u32 i = 0; i < m_MaxFrames; i++)
			memcpy(m_BufferMappings[i], data, size);
	}

	void VulkanUniformBuffer::SetData(const void* data, u32 size, u32 offset)
	{
		VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(RenderContext::Instance().get());
		u32 currentFrame = vulkanContext->GetCurrentFrame();
		memcpy(m_BufferMappings[currentFrame], data, size);
	}

	void VulkanUniformBuffer::SetDataToFrame(u32 frame, const void* data, u32 size, u32 offset)
	{
		memcpy(m_BufferMappings[frame], data, size);
	}
}