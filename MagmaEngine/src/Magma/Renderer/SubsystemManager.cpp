#include "mgmpch.h"
#include "SubsystemManager.h"

#include "Magma/Core/Application.h"
#include "Renderer.h"
#include "RenderSwapchain.h"
#include "Descriptors.h"

#include "Subsystems/DepthPrepassRenderSubsystem.h"
#include "Subsystems/SimpleRenderSubsystem.h"
#include "Subsystems/DefaultRenderSubsystem.h"
#include "Subsystems/SSAORenderSubsystem.h"
#include "Subsystems/SkyboxRenderSubsystem.h"
#include "Subsystems/DrawToBufferRenderSubsystem.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Magma
{
	// TEMPORARY
	static Ref<UniformBuffer> s_SimpleCameraUniformBuffer = nullptr;
	static Ref<UniformBuffer> s_DirLightUniformBuffer = nullptr;

	static Ref<FramebufferTexture2D> s_ColorbufferTexture = nullptr;
	static Ref<FramebufferTexture2D> s_DepthbufferTexture = nullptr;

	static Ref<FramebufferTexture2D> s_PositionsViewTexture = nullptr; // Deferred shading textures
	static Ref<FramebufferTexture2D> s_PositionsWorldTexture = nullptr; // Deferred shading textures
	static Ref<FramebufferTexture2D> s_AlbedoTexture = nullptr;
	static Ref<FramebufferTexture2D> s_NormalMetalRoughnessTexture = nullptr;
	static Ref<FramebufferTexture2D> s_SSAOTextureInput = nullptr;
	static Ref<FramebufferTexture2D> s_SSAOTextureOutput = nullptr;

	static Ref<UniformBuffer> s_CameraUniformBuffer = nullptr; // Camera
	static Ref<UniformBuffer> s_FogUniformBuffer = nullptr; // Fog
	static Ref<UniformBuffer> s_PhysicalCameraUniformBuffer = nullptr; // PhysicalCamera
	static Ref<UniformBuffer> s_LightsUniformBuffer = nullptr; // Lights
	static Ref<Texture2D> s_PreintegratedFG = nullptr; // BRDF-LuT
	static Ref<TextureCube> s_Skybox = nullptr; // BRDF-LuT

	static Ref<UniformBuffer> s_SSAOKernelUniformBuffer = nullptr; // Kernels
	static Ref<Texture2D> s_SSAONoiseTexture = nullptr; // Noise Texture

	static Ref<DescriptorSetLayout> s_DefaultSceneLayout = nullptr;
	static Ref<DescriptorSet> s_DefaultSceneSet = nullptr;
	static Ref<DescriptorSetLayout> s_SimpleSceneLayout = nullptr;
	static Ref<DescriptorSet> s_SimpleSceneSet = nullptr;
	static Ref<DescriptorSetLayout> s_GeometryLayout = nullptr;
	static Ref<DescriptorSet> s_GeometrySet = nullptr;
	static Ref<DescriptorSetLayout> s_SSAOLayout = nullptr;
	static Ref<DescriptorSet> s_SSAOSet = nullptr;
	static Ref<DescriptorSetLayout> s_SSAOBlurLayout = nullptr;
	static Ref<DescriptorSet> s_SSAOBlurSet = nullptr;

	void SubsystemManager::InitSubsystems()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		s_ColorbufferTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::Color, width, height);
		s_DepthbufferTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::Depth, width, height);

		InitUniformBuffers();
		InitTextures();
		InitDescriptorLayouts();

		// TODO: enable subsystem selection
		InitDepthPrepassSubsystem();
#if true
		InitDefaultGBufferSubsystem();
		InitSSAOKernelSubsystem();
		InitSSAOBlurSubsystem();
		InitDefaultDeferredSubsystem();
		InitSimpleDeferredSubsystem();
#else
		InitSimpleSubsystem();
		InitDefaultForwardSubsystem();
#endif
		InitSkyboxSubsystem();
	}

	void SubsystemManager::Shutdown()
	{
		s_DefaultSceneLayout = nullptr;
		s_DefaultSceneSet = nullptr;
		s_SimpleSceneLayout = nullptr;
		s_SimpleSceneSet = nullptr;
		s_GeometryLayout = nullptr;
		s_GeometrySet = nullptr;
		s_SSAOLayout = nullptr;
		s_SSAOSet = nullptr;
		s_SSAOBlurLayout = nullptr;
		s_SSAOBlurSet = nullptr;

		s_CameraUniformBuffer = nullptr;
		s_FogUniformBuffer = nullptr;
		s_PhysicalCameraUniformBuffer = nullptr;
		s_LightsUniformBuffer = nullptr;
		s_PreintegratedFG = nullptr;
		s_Skybox = nullptr;

		s_SSAOKernelUniformBuffer = nullptr;
		s_SSAONoiseTexture = nullptr;

		s_PositionsViewTexture = nullptr;
		s_PositionsWorldTexture = nullptr;
		s_AlbedoTexture = nullptr;
		s_NormalMetalRoughnessTexture = nullptr;
		s_SSAOTextureInput = nullptr;
		s_SSAOTextureOutput = nullptr;

		s_ColorbufferTexture = nullptr;
		s_DepthbufferTexture = nullptr;

		s_SimpleCameraUniformBuffer = nullptr;
		s_DirLightUniformBuffer = nullptr;
	}

	void SubsystemManager::InitDepthPrepassSubsystem()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		const auto& shader = Shader::Create("assets/shaders/DepthPrepass.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.InputElementsLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float3, "a_Normal"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float3, "a_Tangent"},
			{ShaderDataType::Float3, "a_Bitangent"},
		};
		pipelineSpec.GlobalDataLayout.ConstantDataSize = sizeof(DepthPrepassConstantData);
		pipelineSpec.Shader = shader;

		RenderPassSpecification renderPassSpec{};
		renderPassSpec.ClearValues.ClearFlags = ClearFlags::DepthStencil;
		renderPassSpec.ClearValues.Color = { 0.1f, 0.1f, 0.1f, 1.0f };
		renderPassSpec.Attachments = { AttachmentFormat::D32 };
		renderPassSpec.IsSwapchainTarget = false;

		const auto& renderPass = RenderPass::Create(renderPassSpec, device);
		const auto& pipeline = Pipeline::Create(pipelineSpec, device, renderPass);

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { s_DepthbufferTexture };
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Depth } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& depthPrepassRenderSubsystem = CreateRef<DepthPrepassRenderSubsystem>(pipeline);
		Renderer::AddRenderSubsystem(depthPrepassRenderSubsystem);
	}

	void SubsystemManager::InitSimpleSubsystem()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		const auto& shader = Shader::Create("assets/shaders/BasicModelLighting.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.InputElementsLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float3, "a_Normal"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float3, "a_Tangent"},
			{ShaderDataType::Float3, "a_Bitangent"},
		};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_SimpleSceneLayout);
		pipelineSpec.GlobalDataLayout.ConstantDataSize = sizeof(SimpleConstantData);
		pipelineSpec.PipelineDepthState.DepthWrite = false;
		pipelineSpec.PipelineDepthState.DepthFunc = DepthComparison::LessOrEqual;
		pipelineSpec.Shader = shader;

		RenderPassSpecification renderPassSpec{};
		renderPassSpec.ClearValues.ClearFlags = ClearFlags::Color;
		renderPassSpec.ClearValues.Color = { 0.1f, 0.1f, 0.1f, 1.0f };
		renderPassSpec.Attachments = { AttachmentFormat::RGBA8, AttachmentFormat::D32 };
		renderPassSpec.IsSwapchainTarget = false;

		const auto& renderPass = RenderPass::Create(renderPassSpec, device);
		const auto& pipeline = Pipeline::Create(pipelineSpec, device, renderPass);

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { s_ColorbufferTexture, s_DepthbufferTexture };
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Color }, { FramebufferTextureFormat::Depth } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& simpleRenderSubsystem = CreateRef<SimpleRenderSubsystem>(pipeline);
		simpleRenderSubsystem->SetDescriptorChunk({ s_SimpleSceneSet });
		Renderer::AddRenderSubsystem(simpleRenderSubsystem);
		Renderer::SetScreenTexture(s_ColorbufferTexture);
	}

	void SubsystemManager::InitDefaultForwardSubsystem()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		const auto& shader = Shader::Create("assets/shaders/DefaultLighting.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.InputElementsLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float3, "a_Normal"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float3, "a_Tangent"},
			{ShaderDataType::Float3, "a_Bitangent"},
		};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_DefaultSceneLayout);
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(Renderer::GetMaterialDescriptorSetLayout());
		pipelineSpec.GlobalDataLayout.ConstantDataSize = sizeof(DefaultConstantData);
		pipelineSpec.PipelineDepthState.DepthWrite = false;
		pipelineSpec.PipelineDepthState.DepthFunc = DepthComparison::LessOrEqual;
		pipelineSpec.Shader = shader;

		RenderPassSpecification renderPassSpec{};
		renderPassSpec.Attachments = { AttachmentFormat::RGBA8, AttachmentFormat::D32 };
		renderPassSpec.IsSwapchainTarget = false;

		const auto& renderPass = RenderPass::Create(renderPassSpec, device);
		const auto& pipeline = Pipeline::Create(pipelineSpec, device, renderPass);

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { s_ColorbufferTexture, s_DepthbufferTexture };
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Color }, { FramebufferTextureFormat::Depth } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& defaultRenderSubsystem = CreateRef<DefaultRenderSubsystem>(pipeline);
		defaultRenderSubsystem->SetDescriptorChunk({ s_DefaultSceneSet });
		Renderer::AddRenderSubsystem(defaultRenderSubsystem);
		Renderer::SetScreenTexture(s_ColorbufferTexture);
	}

	void SubsystemManager::InitDefaultGBufferSubsystem()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		const auto& shader = Shader::Create("assets/shaders/GBuffer.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.InputElementsLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float3, "a_Normal"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float3, "a_Tangent"},
			{ShaderDataType::Float3, "a_Bitangent"},
		};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_DefaultSceneLayout);
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(Renderer::GetMaterialDescriptorSetLayout());
		pipelineSpec.GlobalDataLayout.ConstantDataSize = sizeof(DefaultConstantData);
		pipelineSpec.Shader = shader;

		RenderPassSpecification renderPassSpec{};
		renderPassSpec.ClearValues.ClearFlags = ClearFlags::Color;
		renderPassSpec.ClearValues.Color = { 0.1f, 0.1f, 0.1f, 1.0f };
		renderPassSpec.Attachments = { AttachmentFormat::RGBA16F, AttachmentFormat::RGBA16F, AttachmentFormat::RGBA8, AttachmentFormat::RGBA16F, AttachmentFormat::D32 };
		renderPassSpec.IsSwapchainTarget = false;

		const auto& renderPass = RenderPass::Create(renderPassSpec, device);
		const auto& pipeline = Pipeline::Create(pipelineSpec, device, renderPass);

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { s_PositionsViewTexture, s_PositionsWorldTexture, s_AlbedoTexture, s_NormalMetalRoughnessTexture, s_DepthbufferTexture };
		framebufferSpec.TextureSpecs = { 
			{ FramebufferTextureFormat::RGBA16F },
			{ FramebufferTextureFormat::RGBA16F },
			{ FramebufferTextureFormat::RGBA8 },
			{ FramebufferTextureFormat::RGBA16F },
			{ FramebufferTextureFormat::Depth }
		};

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& gBufferRenderSubsystem = CreateRef<DefaultRenderSubsystem>(pipeline);
		gBufferRenderSubsystem->EnableNonPBRRendering(true);
		gBufferRenderSubsystem->SetDescriptorChunk({ s_DefaultSceneSet });
		Renderer::AddRenderSubsystem(gBufferRenderSubsystem);
	}

	void SubsystemManager::InitDefaultDeferredSubsystem()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		const auto& shader = Shader::Create("assets/shaders/DefaultDeferred.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_DefaultSceneLayout);
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_GeometryLayout);
		pipelineSpec.GlobalDataLayout.ConstantDataSize = sizeof(DefaultConstantData);
		pipelineSpec.PipelineDepthState.DepthWrite = false;
		pipelineSpec.PipelineDepthState.DepthFunc = DepthComparison::LessOrEqual;
		pipelineSpec.Shader = shader;

		RenderPassSpecification renderPassSpec{};
		renderPassSpec.ClearValues.ClearFlags = ClearFlags::Color;
		renderPassSpec.ClearValues.Color = { 0.1f, 0.1f, 0.1f, 1.0f };
		renderPassSpec.Attachments = { AttachmentFormat::RGBA8, AttachmentFormat::D32 };
		renderPassSpec.IsSwapchainTarget = false;

		const auto& renderPass = RenderPass::Create(renderPassSpec, device);
		const auto& pipeline = Pipeline::Create(pipelineSpec, device, renderPass);

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { s_ColorbufferTexture, s_DepthbufferTexture };
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Color }, { FramebufferTextureFormat::Depth } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& defaultDeferredSubsystem = CreateRef<DrawToBufferRenderSubsystem>(pipeline);
		defaultDeferredSubsystem->SetDescriptorChunk({ s_DefaultSceneSet, s_GeometrySet });
		Renderer::AddRenderSubsystem(defaultDeferredSubsystem);
		Renderer::SetScreenTexture(s_ColorbufferTexture);
	}

	void SubsystemManager::InitSimpleDeferredSubsystem()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		const auto& shader = Shader::Create("assets/shaders/BasicDeferredLighting.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_SimpleSceneLayout);
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_GeometryLayout);
		pipelineSpec.GlobalDataLayout.ConstantDataSize = sizeof(SimpleConstantData);
		pipelineSpec.PipelineDepthState.DepthWrite = false;
		pipelineSpec.PipelineDepthState.DepthFunc = DepthComparison::LessOrEqual;
		pipelineSpec.Shader = shader;

		RenderPassSpecification renderPassSpec{};
		renderPassSpec.Attachments = { AttachmentFormat::RGBA8, AttachmentFormat::D32 };
		renderPassSpec.IsSwapchainTarget = false;

		const auto& renderPass = RenderPass::Create(renderPassSpec, device);
		const auto& pipeline = Pipeline::Create(pipelineSpec, device, renderPass);

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { s_ColorbufferTexture, s_DepthbufferTexture };
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Color }, { FramebufferTextureFormat::Depth } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& defaultDeferredSubsystem = CreateRef<DrawToBufferRenderSubsystem>(pipeline);
		defaultDeferredSubsystem->SetDescriptorChunk({ s_SimpleSceneSet, s_GeometrySet });
		Renderer::AddRenderSubsystem(defaultDeferredSubsystem);
		Renderer::SetScreenTexture(s_ColorbufferTexture);
	}

	void SubsystemManager::InitSSAOKernelSubsystem()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		const auto& shader = Shader::Create("assets/shaders/SSAOKernel.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_SimpleSceneLayout);
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_GeometryLayout);
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_SSAOLayout);
		pipelineSpec.PipelineDepthState.DepthWrite = false;
		pipelineSpec.PipelineDepthState.DepthFunc = DepthComparison::LessOrEqual;
		pipelineSpec.Shader = shader;

		RenderPassSpecification renderPassSpec{};
		renderPassSpec.ClearValues.ClearFlags = ClearFlags::Color;
		renderPassSpec.ClearValues.Color = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassSpec.Attachments = { AttachmentFormat::R8 };
		renderPassSpec.IsSwapchainTarget = false;

		const auto& renderPass = RenderPass::Create(renderPassSpec, device);
		const auto& pipeline = Pipeline::Create(pipelineSpec, device, renderPass);

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { s_SSAOTextureInput };
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Red } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& skyboxSubsystem = CreateRef<SkyboxRenderSubsystem>(pipeline);
		skyboxSubsystem->SetDescriptorChunk({ s_SimpleSceneSet, s_GeometrySet, s_SSAOSet });
		Renderer::AddRenderSubsystem(skyboxSubsystem);
	}

	void SubsystemManager::InitSSAOBlurSubsystem()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		const auto& shader = Shader::Create("assets/shaders/SSAOBlur.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_SSAOBlurLayout);
		pipelineSpec.PipelineDepthState.DepthWrite = false;
		pipelineSpec.PipelineDepthState.DepthFunc = DepthComparison::LessOrEqual;
		pipelineSpec.Shader = shader;

		RenderPassSpecification renderPassSpec{};
		renderPassSpec.ClearValues.ClearFlags = ClearFlags::Color;
		renderPassSpec.ClearValues.Color = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassSpec.Attachments = { AttachmentFormat::R8 };
		renderPassSpec.IsSwapchainTarget = false;

		const auto& renderPass = RenderPass::Create(renderPassSpec, device);
		const auto& pipeline = Pipeline::Create(pipelineSpec, device, renderPass);

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { s_SSAOTextureOutput };
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Red } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& skyboxSubsystem = CreateRef<SkyboxRenderSubsystem>(pipeline);
		skyboxSubsystem->SetDescriptorChunk({ s_SSAOBlurSet });
		Renderer::AddRenderSubsystem(skyboxSubsystem);
	}

	void SubsystemManager::InitSkyboxSubsystem()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		// Scene Layout
		DescriptorBinding cameraTransforms{ DescriptorType::UniformBuffer, 0 };
		DescriptorBinding fogSettings{ DescriptorType::UniformBuffer, 1 };
		DescriptorBinding physicalCamera{ DescriptorType::UniformBuffer, 2 };
		DescriptorBinding lights{ DescriptorType::UniformBuffer, 3 };
		DescriptorBinding brdfLut{ DescriptorType::ImageSampler, 4 };
		DescriptorBinding skybox{ DescriptorType::ImageSampler, 5 };
		DescriptorSetLayoutSpecification sceneLayout{};
		sceneLayout.Bindings = { cameraTransforms, fogSettings, physicalCamera, lights, brdfLut, skybox };

		Ref<DescriptorSetLayout> descriptorLayout = DescriptorSetLayout::Create(sceneLayout, device);
		Ref<DescriptorSet> descriptorSet = DescriptorSet::Create(device, descriptorLayout, Renderer::GetDescriptorPool());

		descriptorSet->WriteUniformBuffer(s_CameraUniformBuffer, (u32)sizeof(GlobalUBO));
		descriptorSet->WriteUniformBuffer(s_FogUniformBuffer, (u32)sizeof(FogUBO));
		descriptorSet->WriteUniformBuffer(s_PhysicalCameraUniformBuffer, (u32)sizeof(PhysicalCameraUBO));
		descriptorSet->WriteUniformBuffer(s_LightsUniformBuffer, (u32)sizeof(LightsUBO));
		descriptorSet->WriteTexture2D(s_PreintegratedFG);
		descriptorSet->WriteTextureCube(s_Skybox);

		const auto& shader = Shader::Create("assets/shaders/Skybox.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.InputElementsLayout = {
			{ ShaderDataType::Float3, "a_SkyboxVertices" }
		};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(descriptorLayout);
		pipelineSpec.PipelineDepthState.DepthWrite = false;
		pipelineSpec.PipelineDepthState.DepthFunc = DepthComparison::LessOrEqual;
		pipelineSpec.Shader = shader;

		RenderPassSpecification renderPassSpec{};
		renderPassSpec.Attachments = { AttachmentFormat::RGBA8, AttachmentFormat::D32 };
		renderPassSpec.IsSwapchainTarget = false;

		const auto& renderPass = RenderPass::Create(renderPassSpec, device);
		const auto& pipeline = Pipeline::Create(pipelineSpec, device, renderPass);

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { Renderer::GetScreenTexture(), s_DepthbufferTexture};
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Color }, { FramebufferTextureFormat::Depth } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& skyboxSubsystem = CreateRef<SkyboxRenderSubsystem>(pipeline);
		skyboxSubsystem->SetDescriptorChunk({ descriptorSet });
		Renderer::AddRenderSubsystem(skyboxSubsystem);
	}

	void SubsystemManager::InitDescriptorLayouts()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		// Default Scene Layout
		DescriptorBinding cameraTransforms{ DescriptorType::UniformBuffer, 0 };
		DescriptorBinding fogSettings{ DescriptorType::UniformBuffer, 1 };
		DescriptorBinding physicalCamera{ DescriptorType::UniformBuffer, 2 };
		DescriptorBinding lights{ DescriptorType::UniformBuffer, 3 };
		DescriptorBinding brdfLut{ DescriptorType::ImageSampler, 4 };
		DescriptorBinding skybox{ DescriptorType::ImageSampler, 5 };
		DescriptorSetLayoutSpecification defaultSceneLayout{};
		defaultSceneLayout.Bindings = { cameraTransforms, fogSettings, physicalCamera, lights, brdfLut, skybox };
		s_DefaultSceneLayout = DescriptorSetLayout::Create(defaultSceneLayout, device);
		s_DefaultSceneSet = DescriptorSet::Create(device, s_DefaultSceneLayout, Renderer::GetDescriptorPool());
		s_DefaultSceneSet->WriteUniformBuffer(s_CameraUniformBuffer, (u32)sizeof(GlobalUBO));
		s_DefaultSceneSet->WriteUniformBuffer(s_FogUniformBuffer, (u32)sizeof(FogUBO));
		s_DefaultSceneSet->WriteUniformBuffer(s_PhysicalCameraUniformBuffer, (u32)sizeof(PhysicalCameraUBO));
		s_DefaultSceneSet->WriteUniformBuffer(s_LightsUniformBuffer, (u32)sizeof(LightsUBO));
		s_DefaultSceneSet->WriteTexture2D(s_PreintegratedFG);
		s_DefaultSceneSet->WriteTextureCube(s_Skybox);

		// Simple Scene Layout
		DescriptorBinding viewProj{ DescriptorType::UniformBuffer, 0 };
		DescriptorBinding dirLight{ DescriptorType::UniformBuffer, 1 };
		DescriptorSetLayoutSpecification simpleSceneLayout{};
		simpleSceneLayout.Bindings = { viewProj, dirLight };
		s_SimpleSceneLayout = DescriptorSetLayout::Create(simpleSceneLayout, device);
		s_SimpleSceneSet = DescriptorSet::Create(device, s_SimpleSceneLayout, Renderer::GetDescriptorPool());
		s_SimpleSceneSet->WriteUniformBuffer(s_SimpleCameraUniformBuffer, sizeof(SimpleCameraUBO));
		s_SimpleSceneSet->WriteUniformBuffer(s_DirLightUniformBuffer, sizeof(SimpleDirLightUBO));

		// Geometry Layout
		DescriptorBinding positionsView{ DescriptorType::ImageSampler, 0 };
		DescriptorBinding positionsWorld{ DescriptorType::ImageSampler, 1 };
		DescriptorBinding albedo{ DescriptorType::ImageSampler, 2 };
		DescriptorBinding normMetRough{ DescriptorType::ImageSampler, 3 };
		DescriptorBinding ssaoTex{ DescriptorType::ImageSampler, 4 };
		DescriptorSetLayoutSpecification geomLayout{};
		geomLayout.Bindings = { positionsView, positionsWorld, albedo, normMetRough, ssaoTex };
		s_GeometryLayout = DescriptorSetLayout::Create(geomLayout, device);
		s_GeometrySet = DescriptorSet::Create(device, s_GeometryLayout, Renderer::GetDescriptorPool());
		s_GeometrySet->WriteFramebufferTexture2D(s_PositionsViewTexture);
		s_GeometrySet->WriteFramebufferTexture2D(s_PositionsWorldTexture);
		s_GeometrySet->WriteFramebufferTexture2D(s_AlbedoTexture);
		s_GeometrySet->WriteFramebufferTexture2D(s_NormalMetalRoughnessTexture);
		s_GeometrySet->WriteFramebufferTexture2D(s_SSAOTextureOutput);

		// SSAO Layout
		DescriptorBinding kernels{ DescriptorType::UniformBuffer, 0 };
		DescriptorBinding noiseTexture{ DescriptorType::ImageSampler, 1 };
		DescriptorSetLayoutSpecification ssaoLayout{};
		ssaoLayout.Bindings = { kernels, noiseTexture };
		s_SSAOLayout = DescriptorSetLayout::Create(ssaoLayout, device);
		s_SSAOSet = DescriptorSet::Create(device, s_SSAOLayout, Renderer::GetDescriptorPool());
		s_SSAOSet->WriteUniformBuffer(s_SSAOKernelUniformBuffer, sizeof(SSAOConstantData));
		s_SSAOSet->WriteTexture2D(s_SSAONoiseTexture);

		DescriptorBinding blur{ DescriptorType::ImageSampler, 0 };
		DescriptorSetLayoutSpecification blurLayout{};
		blurLayout.Bindings = { blur };
		s_SSAOBlurLayout = DescriptorSetLayout::Create(blurLayout, device);
		s_SSAOBlurSet = DescriptorSet::Create(device, s_SSAOBlurLayout, Renderer::GetDescriptorPool());
		s_SSAOBlurSet->WriteFramebufferTexture2D(s_SSAOTextureInput);
	}

	void SubsystemManager::InitUniformBuffers()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		s_CameraUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(GlobalUBO), 0, 2);
		s_FogUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(FogUBO), 1, 2);
		s_PhysicalCameraUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(PhysicalCameraUBO), 2, 2);
		s_LightsUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(LightsUBO), 3, 2);

		GlobalUBO sceneUbo{};
		sceneUbo.viewProj = Camera::Main->GetViewProjection();
		sceneUbo.view = Camera::Main->GetView();
		sceneUbo.proj = Camera::Main->GetProjection();
		s_CameraUniformBuffer->SetCommonDataForAllFrames(&sceneUbo, sizeof(sceneUbo));

		PhysicalCameraUBO physCam{};
		physCam.focalLength = 28.0f;
		physCam.focusingDistance = 5.5f;
		physCam.exposure = exp(0.0f);
		physCam.aperture = 5.6f;
		s_PhysicalCameraUniformBuffer->SetCommonDataForAllFrames(&physCam, sizeof(physCam));

		FogUBO fog{};
		fog.fogColor = glm::vec4(0.01f, 0.01f, 0.01f, 0.0f);
		s_FogUniformBuffer->SetCommonDataForAllFrames(&fog, sizeof(fog));

		LightsUBO lightsUbo{};
		DirLight dirLight;
		dirLight.color = { 1.0f, 0.99f, 0.96f, 1.0f };
		dirLight.direction = glm::vec3(-1.0f);
		dirLight.intensity = 10.0f;
		lightsUbo.dirLight[0] = dirLight;
		s_LightsUniformBuffer->SetCommonDataForAllFrames(&lightsUbo, sizeof(lightsUbo));

		s_SimpleCameraUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(SimpleCameraUBO), 0, 2);
		s_DirLightUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(SimpleDirLightUBO), 1, 2);

		SimpleCameraUBO cameraData{};
		cameraData.view = Camera::Main->GetView();
		cameraData.proj = Camera::Main->GetProjection();
		cameraData.viewProj = Camera::Main->GetViewProjection();

		SimpleDirLightUBO dirLightData{};
		dirLightData.color = { 1.0f, 0.99f, 0.96f, 1.0f };
		dirLightData.direction = { -1.0f, -1.0f, -1.0f };
		dirLightData.intensity = 1.0f;

		s_SimpleCameraUniformBuffer->SetCommonDataForAllFrames(&cameraData, sizeof(SimpleCameraUBO));
		s_DirLightUniformBuffer->SetCommonDataForAllFrames(&dirLightData, sizeof(SimpleDirLightUBO));

		s_SSAOKernelUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(SSAOConstantData), 0, 2);
		SSAOConstantData ssaoData{};
		SSAORenderSubsystem::CalculateKernelPositions(ssaoData);
		s_SSAOKernelUniformBuffer->SetCommonDataForAllFrames(&ssaoData, (u32)sizeof(SSAOConstantData));
	}

	void SubsystemManager::InitTextures()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		s_PositionsViewTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::RGBA16F, width, height);
		s_PositionsWorldTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::RGBA16F, width, height);
		s_AlbedoTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::RGBA8, width, height);
		s_NormalMetalRoughnessTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::RGBA16F, width, height);
		s_SSAOTextureInput = FramebufferTexture2D::Create(device, FramebufferTextureFormat::Red, width, height);
		s_SSAOTextureOutput = FramebufferTexture2D::Create(device, FramebufferTextureFormat::Red, width, height);

		s_PositionsWorldTexture->SetBinding(1);
		s_AlbedoTexture->SetBinding(2);
		s_NormalMetalRoughnessTexture->SetBinding(3);
		s_SSAOTextureOutput->SetBinding(4);

		TextureSpecs specs{};
		specs.Filter = TextureSpecs::Filtering::Nearest;
		s_SSAONoiseTexture = Texture2D::Create(device, "assets/textures/ssao-noise.jpg", specs);
		s_SSAONoiseTexture->SetBinding(1);

		s_PreintegratedFG = Texture2D::Create(device, "assets/textures/PreintegratedFG.bmp");
		s_PreintegratedFG->SetBinding(4); // From SceneSet.glslh

		const std::array<std::string, 6> skyboxLayers =
		{
			"assets/textures/skybox/right.jpg",
			"assets/textures/skybox/left.jpg",
			"assets/textures/skybox/up.jpg",
			"assets/textures/skybox/down.jpg",
			"assets/textures/skybox/front.jpg",
			"assets/textures/skybox/back.jpg",
		};
		s_Skybox = TextureCube::Create(device, skyboxLayers);
		s_Skybox->SetBinding(5); // From SceneSet.glslh
	}

	void SubsystemManager::Update()
	{
		GlobalUBO sceneUbo{};
		sceneUbo.viewProj = Camera::Main->GetViewProjection();
		sceneUbo.view = Camera::Main->GetView();
		sceneUbo.proj = Camera::Main->GetProjection();
		s_CameraUniformBuffer->SetData(&sceneUbo, sizeof(sceneUbo));

		SimpleCameraUBO simpleCameraUbo{};
		simpleCameraUbo.viewProj = Camera::Main->GetViewProjection();
		simpleCameraUbo.view = Camera::Main->GetView();
		simpleCameraUbo.proj = Camera::Main->GetProjection();
		s_SimpleCameraUniformBuffer->SetData(&simpleCameraUbo, sizeof(simpleCameraUbo));
	}
}