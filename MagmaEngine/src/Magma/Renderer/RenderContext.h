#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderPass.h"

#include "Magma/Renderer/Pipeline.h"
#include "Magma/Renderer/VertexBuffer.h"
#include "Magma/Renderer/IndexBuffer.h"
#include "Magma/Renderer/Descriptors.h"

namespace Magma
{
	enum class RenderAPI
	{
		None,
		Vulkan
	};

	class MAGMA_API RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void Present() = 0;

		virtual void SetViewport(u32 x, u32 y, u32 width, u32 height) = 0;
		virtual void SetScissor(i32 x, i32 y, u32 width, u32 height) = 0;
		virtual void BeginRenderPass(const Ref<RenderPass>& renderPass) = 0;
		virtual void EndRenderPass(const Ref<RenderPass>& renderPass) = 0;

		virtual void BindPipeline(const Ref<Pipeline>& pipeline) = 0;
		virtual void BindVertexBuffer(const Ref<VertexBuffer>& pipeline) = 0;
		virtual void BindIndexBuffer(const Ref<IndexBuffer>& pipeline) = 0;
		virtual void BindDescriptorSet(const Ref<DescriptorSet>& descriptorSet, const Ref<Pipeline>& pipeline, u32 firstSet) = 0;
		virtual void UploadConstantData(const Ref<Pipeline>& pipeline, const u32 size, const void* data) = 0;
		virtual void DrawVertices(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) = 0;
		virtual void DrawIndices(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 firstInstance, i32 vertexOffset = 0) = 0;

		inline static RenderAPI GetAPI() { return s_API; }
		static Scope<RenderContext> Create();
		static const Scope<RenderContext>& Instance();

	private:
		static RenderAPI s_API;
	};
}

