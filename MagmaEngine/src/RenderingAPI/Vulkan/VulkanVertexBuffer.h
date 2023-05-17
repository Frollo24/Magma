#pragma once
#include "Magma/Renderer/VertexBuffer.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const Ref<RenderDevice>& device, u32 size, const void* data);
		virtual ~VulkanVertexBuffer();

		inline VkBuffer GetHandle() { return m_Buffer; }

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_BufferMemory = VK_NULL_HANDLE;
	};
}

