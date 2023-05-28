#include "mgmpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "SubsystemManager.h"

#include "Magma/Core/Application.h"
#include "Magma/Renderer/RenderSwapchain.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Magma
{
	bool Renderer::s_BegunFrame = false;
	Ref<Camera> Renderer::s_Camera = nullptr;
	std::vector<Ref<RenderSubsystem>> Renderer::s_RenderSubsystems{};

	struct RendererData
	{
		Ref<RenderDevice> RenderDevice = nullptr;
		Ref<RenderSwapchain> RenderSwapchain = nullptr;
		Ref<DescriptorPool> DescriptorPool = nullptr;
		Ref<Shader> ScreenShader = nullptr;
		Ref<Pipeline> ScreenPipeline = nullptr;
		Ref<DescriptorSetLayout> ScreenDescriptorLayout = nullptr;
		Ref<DescriptorSet> ScreenDescriptorSet = nullptr;
		Ref<FramebufferTexture2D> ScreenTexture = nullptr;
		Ref<DescriptorSetLayout> MaterialDescriptorLayout = nullptr;
	};

	static RendererData* s_RendererData = nullptr;

	void Renderer::Init()
	{
		Camera::Main = CreateRef<Camera>(glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.3f, 50.0f));
		Camera::Main->SetView(glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
		s_Camera = Camera::Main;

		s_RendererData = new RendererData();
		s_RendererData->RenderDevice = Application::Instance().GetWindow().GetGraphicsInstance()->GetDevice();
		s_RendererData->RenderSwapchain = Application::Instance().GetWindow().GetGraphicsInstance()->GetSwapchain();
		RenderCommand::Init();

		// TEMPORARY
		RenderPassSpecification swapchainSpec;
		swapchainSpec.ClearValues.ClearFlags = ClearFlags::Color;
		swapchainSpec.ClearValues.Color = { 1.0f, 0.0f, 0.0f, 1.0f };
		swapchainSpec.Attachments = { AttachmentFormat::RGBA8 };
		swapchainSpec.IsSwapchainTarget = true;

		auto renderPass = RenderPass::Create(swapchainSpec, s_RendererData->RenderDevice);
		s_RendererData->RenderSwapchain->CreateFramebuffers(s_RendererData->RenderDevice, renderPass);

		DescriptorBinding screenTexture{ DescriptorType::ImageSampler, 0 };
		DescriptorSetLayoutSpecification layoutSpec{};
		layoutSpec.Bindings = { screenTexture };
		s_RendererData->DescriptorPool = DescriptorPool::Create(s_RendererData->RenderDevice);
		s_RendererData->ScreenDescriptorLayout = DescriptorSetLayout::Create(layoutSpec, s_RendererData->RenderDevice);
		s_RendererData->ScreenDescriptorSet = DescriptorSet::Create(s_RendererData->RenderDevice, s_RendererData->ScreenDescriptorLayout, s_RendererData->DescriptorPool);

		s_RendererData->ScreenShader = Shader::Create("assets/shaders/DrawToScreen.glsl");
		PipelineSpecification spec{};
		spec.GlobalDataLayout.DescriptorLayouts.push_back(s_RendererData->ScreenDescriptorLayout);
		spec.Shader = s_RendererData->ScreenShader;
		s_RendererData->ScreenPipeline = Pipeline::Create(spec, s_RendererData->RenderDevice, renderPass);

		// Material Layout
		DescriptorBinding albedoTexture{ DescriptorType::ImageSampler, 0 };
		DescriptorBinding normalTexture{ DescriptorType::ImageSampler, 1 };
		DescriptorBinding metallicTexture{ DescriptorType::ImageSampler, 2 };
		DescriptorBinding roughnessTexture{ DescriptorType::ImageSampler, 3 };
		DescriptorBinding emmisiveTexture{ DescriptorType::ImageSampler, 4 };

		DescriptorSetLayoutSpecification materialLayout;
		materialLayout.Bindings = { albedoTexture, normalTexture, metallicTexture, roughnessTexture, emmisiveTexture };
		s_RendererData->MaterialDescriptorLayout = DescriptorSetLayout::Create(materialLayout, s_RendererData->RenderDevice);

		// Init subsystems
		SubsystemManager::InitSubsystems();
	}

	void Renderer::BeginFrame()
	{
		MGM_CORE_ASSERT(!s_BegunFrame, "Frame already has begun! Did you forget to call Renderer::EndFrame?");
		s_BegunFrame = true;

		RenderCommand::BeginFrame();
	}

	void Renderer::RenderGameObjects()
	{
		for (const auto& system : s_RenderSubsystems)
		{
			const auto& renderPass = system->GetPipeline()->GetRenderPass();
			const bool& isSwapchainTarget = renderPass->GetSpecification().IsSwapchainTarget;
			u32 width = isSwapchainTarget ? s_RendererData->RenderSwapchain->GetWidth() : renderPass->GetFramebuffer()->GetSpecification().Width;
			u32 height = isSwapchainTarget ? s_RendererData->RenderSwapchain->GetHeight() : renderPass->GetFramebuffer()->GetSpecification().Height;

			RenderCommand::BeginRenderPass(renderPass);
			RenderCommand::SetViewport(0, 0, width, height);
			RenderCommand::SetScissor(0, 0, width, height);
			system->Bind();
			system->RenderGameObjects();
			RenderCommand::EndRenderPass(renderPass);
		}
	}

	void Renderer::DrawToScreen()
	{
		auto drawToScreenPass = s_RendererData->RenderSwapchain->GetMainRenderPass();
		u32 width = s_RendererData->RenderSwapchain->GetWidth();
		u32 height = s_RendererData->RenderSwapchain->GetHeight();

		RenderCommand::BeginRenderPass(drawToScreenPass);
		RenderCommand::SetViewport(0, 0, width, height);
		RenderCommand::SetScissor(0, 0, width, height);
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

		SubsystemManager::Shutdown();

		for (auto& subsystem : s_RenderSubsystems)
			subsystem = nullptr;
	}

	void Renderer::AddGameObject(const Ref<GameObject>& gameObject)
	{
		bool wasAdded = false;

		for (const auto& subsystem : Renderer::GetRenderSubsystems())
			if (subsystem->TryAddGameObject(gameObject))
				wasAdded = true;

		if (!wasAdded)
			MGM_CORE_WARN("A gameObject could not be added to the renderer!");
	}

	const Ref<FramebufferTexture2D>& Renderer::GetScreenTexture()
	{
		return s_RendererData->ScreenTexture;
	}

	void Renderer::SetScreenTexture(const Ref<FramebufferTexture2D>& screenTexture)
	{
		s_RendererData->ScreenTexture = screenTexture;
		s_RendererData->ScreenDescriptorSet->WriteFramebufferTexture2D(screenTexture);
	}

	const Ref<DescriptorPool>& Renderer::GetDescriptorPool()
	{
		return s_RendererData->DescriptorPool;
	}

	const Ref<DescriptorSetLayout>& Renderer::GetMaterialDescriptorSetLayout()
	{
		return s_RendererData->MaterialDescriptorLayout;
	}

	const Ref<RenderDevice>& Renderer::GetDevice()
	{
		return s_RendererData->RenderDevice;
	}

	void Renderer::Update()
	{
		SubsystemManager::Update();
	}
}