#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderContext.h"

namespace Magma
{
	class RenderCommand
	{
	public:
		static void Init();
		static void SetViewport(u32 x, u32 y, u32 width, u32 height);
		static void SetScissor(i32 x, i32 y, u32 width, u32 height);
		static void Shutdown();

	private:
		static Scope<RenderContext> s_RenderContext;
	};
}

