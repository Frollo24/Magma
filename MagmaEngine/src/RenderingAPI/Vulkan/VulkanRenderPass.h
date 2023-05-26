#pragma once
#include "Magma/Renderer/RenderPass.h"

#include "VulkanFramebuffer.h"
#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& spec, const Ref<RenderDevice>& device);
		virtual ~VulkanRenderPass();

		inline virtual u32 GetColorAttachmentsCount() { return m_ColorAttachmentCount; }

		inline VkRenderPass GetHandle() const { return m_RenderPass; }

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		u32 m_ColorAttachmentCount = 0;
	};
}

