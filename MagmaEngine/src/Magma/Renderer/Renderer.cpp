#include "mgmpch.h"
#include "Renderer.h"
#include "RenderCommand.h"

#include "Magma/Core/Application.h"

namespace Magma
{
	bool Renderer::s_BegunFrame = false;

	struct RendererData
	{
		Ref<RenderDevice> RenderDevice = nullptr;
	};

	static RendererData* s_RendererData = nullptr;

	void Renderer::Init()
	{
		s_RendererData = new RendererData();
		s_RendererData->RenderDevice = Application::Instance().GetWindow().GetGraphicsInstance()->GetDevice();
		RenderCommand::Init();

		// TEMPORARY
		RenderPassSpecification swapchainSpec;
		swapchainSpec.Attachments = { AttachmentFormat::RGBA8 };
		swapchainSpec.IsSwapchainTarget = true;

		auto renderPass = RenderPass::Create(swapchainSpec, s_RendererData->RenderDevice);
	}

	void Renderer::BeginFrame()
	{
		MGM_CORE_ASSERT(!s_BegunFrame, "Frame already has begun! Did you forget to call Renderer::EndFrame?");
		s_BegunFrame = true;

		RenderCommand::BeginFrame();
	}

	void Renderer::EndFrame()
	{
		MGM_CORE_ASSERT(s_BegunFrame, "Didn't begin any frame! Did you forget to call Renderer::BeginFrame?");
		s_BegunFrame = false;

		RenderCommand::EndFrame();
	}

	void Renderer::Shutdown()
	{
		delete s_RendererData;
		RenderCommand::Shutdown();
	}
}