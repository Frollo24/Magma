#pragma once
#include "Magma/Renderer/IndexBuffer.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	static VkIndexType IndexSizeToVkIndexType(const IndexSize& size)
	{
		switch (size)
		{
			case IndexSize::U16: return VK_INDEX_TYPE_UINT16;
			case IndexSize::U32: return VK_INDEX_TYPE_UINT32;
			default:
				return VK_INDEX_TYPE_NONE_KHR;
		}
	}

	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const Ref<RenderDevice>& device, u32 count, const void* indices, const IndexSize& indexSize = IndexSize::U32);
		virtual ~VulkanIndexBuffer();

		inline VkBuffer GetHandle() { return m_Buffer; }

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_BufferMemory = VK_NULL_HANDLE;
	};
}

