#include "mgmpch.h"
#include "Renderer.h"
#include "RenderCommand.h"

#include "Magma/Core/Application.h"
#include "Magma/Renderer/RenderSwapchain.h"

namespace Magma
{
	bool Renderer::s_BegunFrame = false;

	struct RendererData
	{
		Ref<RenderDevice> RenderDevice = nullptr;
		Ref<RenderSwapchain> RenderSwapchain = nullptr;
		Ref<DescriptorPool> DescriptorPool = nullptr;
		Ref<Shader> ScreenShader = nullptr;
		Ref<Pipeline> ScreenPipeline = nullptr;
		Ref<DescriptorSetLayout> ScreenDescriptorLayout = nullptr;
		Ref<DescriptorSet> ScreenDescriptorSet = nullptr;
	};

	static RendererData* s_RendererData = nullptr;

	void Renderer::Init()
	{
		s_RendererData = new RendererData();
		s_RendererData->RenderDevice = Application::Instance().GetWindow().GetGraphicsInstance()->GetDevice();
		s_RendererData->RenderSwapchain = Application::Instance().GetWindow().GetGraphicsInstance()->GetSwapchain();
		RenderCommand::Init();

		// TEMPORARY
		RenderPassSpecification swapchainSpec;
		swapchainSpec.ClearValues.ClearFlags |= ClearFlags::Color;
		swapchainSpec.ClearValues.Color = { 1.0f, 0.0f, 0.0f, 1.0f };
		swapchainSpec.Attachments = { AttachmentFormat::RGBA8 };
		swapchainSpec.IsSwapchainTarget = true;

		auto renderPass = RenderPass::Create(swapchainSpec, s_RendererData->RenderDevice);
		s_RendererData->RenderSwapchain->CreateFramebuffers(s_RendererData->RenderDevice, renderPass);

		Magma::DescriptorBinding screenTexture{ Magma::DescriptorType::ImageSampler, 0 };
		Magma::DescriptorSetLayoutSpecification layoutSpec{};
		layoutSpec.Bindings = { screenTexture };
		s_RendererData->DescriptorPool = Magma::DescriptorPool::Create(s_RendererData->RenderDevice);
		s_RendererData->ScreenDescriptorLayout = Magma::DescriptorSetLayout::Create(layoutSpec, s_RendererData->RenderDevice);
		s_RendererData->ScreenDescriptorSet = Magma::DescriptorSet::Create(s_RendererData->RenderDevice, s_RendererData->ScreenDescriptorLayout, s_RendererData->DescriptorPool);

		s_RendererData->ScreenShader = Magma::Shader::Create("assets/shaders/DrawToScreen.glsl");
		Magma::PipelineSpecification spec{};
		spec.GlobalDataLayout.DescriptorLayouts.push_back(s_RendererData->ScreenDescriptorLayout);
		spec.Shader = s_RendererData->ScreenShader;

		s_RendererData->ScreenPipeline = Magma::Pipeline::Create(spec, s_RendererData->RenderDevice, renderPass);
	}

	void Renderer::BeginFrame()
	{
		MGM_CORE_ASSERT(!s_BegunFrame, "Frame already has begun! Did you forget to call Renderer::EndFrame?");
		s_BegunFrame = true;

		RenderCommand::BeginFrame();
	}

	void Renderer::DrawToScreen()
	{
		auto drawToScreenPass = s_RendererData->RenderSwapchain->GetMainRenderPass();

		RenderCommand::BeginRenderPass(drawToScreenPass);
		RenderCommand::BindPipeline(s_RendererData->ScreenPipeline);
		RenderCommand::BindDescriptorSet(s_RendererData->ScreenDescriptorSet, s_RendererData->ScreenPipeline, 0);
		RenderCommand::DrawVertices(6, 1, 0, 0);
		RenderCommand::EndRenderPass(drawToScreenPass);
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

	void Renderer::SetScreenTexture(const Ref<FramebufferTexture2D>& screenTexture)
	{
		s_RendererData->ScreenDescriptorSet->WriteFramebufferTexture2D(screenTexture);
	}
}