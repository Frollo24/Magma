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

	void RenderCommand::BindVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		s_RenderContext->BindVertexBuffer(vertexBuffer);
	}

	void RenderCommand::BindIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		s_RenderContext->BindIndexBuffer(indexBuffer);
	}

	void RenderCommand::BindDescriptorSet(const Ref<DescriptorSet>& descriptorSet, const Ref<Pipeline>& pipeline, u32 firstSet)
	{
		s_RenderContext->BindDescriptorSet(descriptorSet, pipeline, firstSet);
	}

	void RenderCommand::UploadConstantData(const Ref<Pipeline>& pipeline, const u32 size, const void* data)
	{
		s_RenderContext->UploadConstantData(pipeline, size, data);
	}

	void RenderCommand::DrawVertices(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
	{
		s_RenderContext->DrawVertices(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void RenderCommand::DrawIndices(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 firstInstance, i32 vertexOffset)
	{
		s_RenderContext->DrawIndices(indexCount, instanceCount, firstIndex, firstInstance, vertexOffset);
	}
}