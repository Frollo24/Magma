#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderContext.h"

namespace Magma
{
	class MAGMA_API RenderCommand
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame();
		static void Present();

		static void SetViewport(u32 x, u32 y, u32 width, u32 height);
		static void SetScissor(i32 x, i32 y, u32 width, u32 height);
		static void BeginRenderPass(const Ref<RenderPass>& renderPass);
		static void EndRenderPass(const Ref<RenderPass>& renderPass);

		static void BindPipeline(const Ref<Pipeline>& pipeline);
		static void BindVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
		static void BindIndexBuffer(const Ref<IndexBuffer>& indexBuffer);
		static void UploadConstantData(const Ref<Pipeline>& pipeline, const u32 size, const void* data);
		static void DrawVertices(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
		static void DrawIndices(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 firstInstance, i32 vertexOffset = 0);

	private:
		friend class RenderContext;
		static Scope<RenderContext> s_RenderContext;
	};
}

