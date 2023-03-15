#pragma once
#include "Magma/Renderer/Framebuffer.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec, const Ref<RenderDevice>& device, const Ref<RenderPass>& renderPass);
		virtual ~VulkanFramebuffer();

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkExtent2D m_Extent{};
	};
}

