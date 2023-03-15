#include "mgmpch.h"
#include "VulkanFramebuffer.h"

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"

namespace Magma
{
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec, const Ref<RenderDevice>& device, const Ref<RenderPass>& renderPass)
		: Framebuffer(spec), m_Device(DynamicCast<VulkanDevice>(device)->GetLogicalDevice()),
		m_RenderPass(DynamicCast<VulkanRenderPass>(renderPass)->GetHandle()), m_Extent{ spec.Width, spec.Height }
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = static_cast<u32>(m_Specification.TextureSpecs.size());
		framebufferInfo.pAttachments = m_Specification.ImageViews.data();
		framebufferInfo.width = m_Extent.width;
		framebufferInfo.height = m_Extent.height;
		framebufferInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_Framebuffer);
		MGM_CORE_VERIFY(result == VK_SUCCESS);

		MGM_CORE_INFO("Successfully created a Vulkan Framebuffer!");
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		vkDestroyFramebuffer(m_Device, m_Framebuffer, nullptr);
	}
}
