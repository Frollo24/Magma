#pragma once
#include "Magma/Renderer/UniformBuffer.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(const Ref<RenderDevice>& device, u32 size, u32 binding, u32 maxFrames);
		virtual ~VulkanUniformBuffer();

		inline const VkBuffer& GetHandle(u32 frame) { return m_Buffers[frame]; }

		virtual void SetCommonDataForAllFrames(const void* data, u32 size, u32 offset = 0) override;
		virtual void SetData(const void* data, u32 size, u32 offset = 0) override;
		virtual void SetDataToFrame(u32 frame, const void* data, u32 size, u32 offset = 0) override;

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		std::vector<VkBuffer> m_Buffers{};
		std::vector<VkDeviceMemory> m_BufferMemories{};
		std::vector<void*> m_BufferMappings{};
	};
}

