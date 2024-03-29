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

		virtual void BindPipeline(const Ref<Pipeline>& pipeline) override;
		virtual void BindVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void BindIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;
		virtual void BindDescriptorSet(const Ref<DescriptorSet>& descriptorSet, const Ref<Pipeline>& pipeline, u32 firstSet) override;
		virtual void UploadConstantData(const Ref<Pipeline>& pipeline, const u32 size, const void* data) override;
		virtual void DrawVertices(u32 vertexCount, u32 instanceCount, u32 firstVertex = 0, u32 firstInstance = 0) override;
		virtual void DrawIndices(u32 indexCount, u32 instanceCount, u32 firstIndex = 0, u32 firstInstance = 0, i32 vertexOffset = 0) override;

		inline const VkCommandPool& GetCommandPool() { return m_CommandPool; }
		inline const u32& GetCurrentFrame() { return m_CurrentFrame; }

	private:
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers{};
		VkCommandBuffer m_ActiveCommandBuffer = VK_NULL_HANDLE;

		Ref<RenderPass> m_ActiveRenderPass = nullptr;
		u32 m_TotalFrames = 0, m_CurrentFrame = 0;
	};
}

