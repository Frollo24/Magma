#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderPass.h"

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

		virtual void SetViewport(u32 x, u32 y, u32 width, u32 height) = 0;
		virtual void SetScissor(i32 x, i32 y, u32 width, u32 height) = 0;
		virtual void BeginRenderPass(const Ref<RenderPass>& renderPass) = 0;
		virtual void EndRenderPass(const Ref<RenderPass>& renderPass) = 0;

		inline static RenderAPI GetAPI() { return s_API; }
		static Scope<RenderContext> Create();

	private:
		static RenderAPI s_API;
	};
}
