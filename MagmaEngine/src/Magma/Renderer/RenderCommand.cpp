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

	void RenderCommand::Shutdown()
	{
		s_RenderContext->Shutdown();
		s_RenderContext = nullptr;
	}

	void RenderCommand::BeginFrame()
	{
		s_RenderContext->BeginFrame();
	}

	void RenderCommand::EndFrame()
	{
		s_RenderContext->EndFrame();
	}

	void RenderCommand::Present()
	{
		s_RenderContext->Present();
	}

	void RenderCommand::SetViewport(u32 x, u32 y, u32 width, u32 height)
	{
		s_RenderContext->SetViewport(x, y, width, height);
	}

	void RenderCommand::SetScissor(i32 x, i32 y, u32 width, u32 height)
	{
		s_RenderContext->SetScissor(x, y, width, height);
	}

	void RenderCommand::BeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		s_RenderContext->BeginRenderPass(renderPass);
	}

	void RenderCommand::EndRenderPass(const Ref<RenderPass>& renderPass)
	{
		s_RenderContext->EndRenderPass(renderPass);
	}

	void RenderCommand::BindPipeline(const Ref<Pipeline>& pipeline)
	{
		s_RenderContext->BindPipeline(pipeline);
	}

	void RenderCommand::DrawVertices(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
	{
		s_RenderContext->DrawVertices(vertexCount, instanceCount, firstVertex, firstInstance);
	}
}