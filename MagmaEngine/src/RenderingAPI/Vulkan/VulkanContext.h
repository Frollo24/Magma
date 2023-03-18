#pragma once
#include "Magma/Renderer/RenderContext.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanContext : public RenderContext
	{
	public:
		VulkanContext() = default;
		virtual ~VulkanContext() = default;

		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void Present() override;

		virtual void SetViewport(u32 x, u32 y, u32 width, u32 height) override;
		virtual void SetScissor(i32 x, i32 y, u32 width, u32 height) override;
		virtual void BeginRenderPass(const Ref<RenderPass>& renderPass) override;
		virtual void EndRenderPass(const Ref<RenderPass>& renderPass) override;

	private:
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers{};
		VkCommandBuffer m_ActiveCommandBuffer = VK_NULL_HANDLE;

		Ref<RenderPass> m_ActiveRenderPass = nullptr;
		u32 m_TotalFrames = 0, m_CurrentFrame = 0;
	};
}

