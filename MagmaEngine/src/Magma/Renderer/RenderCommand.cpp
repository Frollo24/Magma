#include "mgmpch.h"
#include "RenderCommand.h"

namespace Magma
{
	Scope<RenderContext> RenderCommand::s_RenderContext = nullptr;

	void RenderCommand::Init()
	{
		s_RenderContext = RenderContext::Create();
		s_RenderContext->Init();
	}

	void RenderCommand::SetViewport(u32 x, u32 y, u32 width, u32 height)
	{
		s_RenderContext->SetViewport(x, y, width, height);
	}

	void RenderCommand::SetScissor(i32 x, i32 y, u32 width, u32 height)
	{
		s_RenderContext->SetScissor(x, y, width, height);
	}

	void RenderCommand::Shutdown()
	{
		s_RenderContext->Shutdown();
	}
}