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
		static void DrawVertices(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);

	private:
		static Scope<RenderContext> s_RenderContext;
	};
}

