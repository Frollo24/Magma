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

		inline VkRenderPass GetHandle() const { return m_RenderPass; }

		inline const Ref<VulkanFramebuffer>& GetFramebuffer() { return m_Framebuffer; }
		inline void SetFramebuffer(const Ref<VulkanFramebuffer>& framebuffer) { m_Framebuffer = framebuffer; }

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		Ref<VulkanFramebuffer> m_Framebuffer = nullptr;
	};
}

