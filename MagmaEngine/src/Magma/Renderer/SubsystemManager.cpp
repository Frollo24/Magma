#include "mgmpch.h"
#include "SubsystemManager.h"

#include "Magma/Core/Application.h"
#include "Renderer.h"
#include "RenderSwapchain.h"
#include "Descriptors.h"

#include "Subsystems/DepthPrepassRenderSubsystem.h"
#include "Subsystems/SimpleRenderSubsystem.h"
#include "Subsystems/DefaultRenderSubsystem.h"
#include "Subsystems/DrawToBufferRenderSubsystem.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Magma
{
	// TEMPORARY
	static Ref<UniformBuffer> s_SimpleCameraUniformBuffer = nullptr;
	static Ref<UniformBuffer> s_DirLightUniformBuffer = nullptr;

	static Ref<FramebufferTexture2D> s_DepthbufferTexture = nullptr;

	static Ref<FramebufferTexture2D> s_PositionsViewTexture = nullptr; // Deferred shading textures
	static Ref<FramebufferTexture2D> s_PositionsWorldTexture = nullptr; // Deferred shading textures
	static Ref<FramebufferTexture2D> s_AlbedoTexture = nullptr;
	static Ref<FramebufferTexture2D> s_NormalMetalRoughnessTexture = nullptr;

	static Ref<UniformBuffer> s_CameraUniformBuffer = nullptr; // Camera
	static Ref<UniformBuffer> s_FogUniformBuffer = nullptr; // Fog
	static Ref<UniformBuffer> s_PhysicalCameraUniformBuffer = nullptr; // PhysicalCamera
	static Ref<UniformBuffer> s_LightsUniformBuffer = nullptr; // Lights
	static Ref<Texture2D> s_PreintegratedFG = nullptr; // BRDF-LuT
	static Ref<TextureCube> s_Skybox = nullptr; // BRDF-LuT

	void SubsystemManager::InitSubsystems()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		s_DepthbufferTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::Depth, width, height);

		InitUniformBuffers();
		InitTextures();

		// TODO: enable subsystem selection
		InitDepthPrepassSubsystem();
		// InitSimpleSubsystem();
#if false
		InitDefaultGBufferSubsystem();
		InitDefaultDeferredSubsystem();
#else
		InitDefaultForwardSubsystem();
#endif
	}

	void SubsystemManager::Shutdown()
	{
		s_CameraUniformBuffer = nullptr;
		s_FogUniformBuffer = nullptr;
		s_PhysicalCameraUniformBuffer = nullptr;
		s_LightsUniformBuffer = nullptr;
		s_PreintegratedFG = nullptr;
		s_Skybox = nullptr;

		s_PositionsViewTexture = nullptr;
		s_PositionsWorldTexture = nullptr;
		s_AlbedoTexture = nullptr;
		s_NormalMetalRoughnessTexture = nullptr;

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

		DescriptorBinding viewProj{ DescriptorType::UniformBuffer, 0 };
		DescriptorBinding dirLight{ DescriptorType::UniformBuffer, 1 };
		DescriptorSetLayoutSpecification layoutSpec{};
		layoutSpec.Bindings = { viewProj, dirLight };
		Ref<DescriptorSetLayout> descriptorLayout = DescriptorSetLayout::Create(layoutSpec, device);
		Ref<DescriptorSet> descriptorSet = DescriptorSet::Create(device, descriptorLayout, Renderer::GetDescriptorPool());

		s_SimpleCameraUniformBuffer = UniformBuffer::Create(device, sizeof(SimpleCameraUBO), 0, 2);
		s_DirLightUniformBuffer = UniformBuffer::Create(device, sizeof(SimpleDirLightUBO), 1, 2);
		descriptorSet->WriteUniformBuffer(s_SimpleCameraUniformBuffer, sizeof(SimpleCameraUBO));
		descriptorSet->WriteUniformBuffer(s_DirLightUniformBuffer, sizeof(SimpleDirLightUBO));

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

		const auto& shader = Shader::Create("assets/shaders/BasicModelLighting.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.InputElementsLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float3, "a_Normal"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float3, "a_Tangent"},
			{ShaderDataType::Float3, "a_Bitangent"},
		};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(descriptorLayout);
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
		const auto& framebufferTextureColor = FramebufferTexture2D::Create(device, FramebufferTextureFormat::Color, width, height);
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { framebufferTextureColor, s_DepthbufferTexture };
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Color }, { FramebufferTextureFormat::Depth } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& simpleRenderSubsystem = CreateRef<SimpleRenderSubsystem>(pipeline);
		simpleRenderSubsystem->SetDescriptorChunk({ descriptorSet });
		Renderer::AddRenderSubsystem(simpleRenderSubsystem);
		Renderer::SetScreenTexture(framebufferTextureColor);
	}

	void SubsystemManager::InitDefaultForwardSubsystem()
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

		const auto& shader = Shader::Create("assets/shaders/DefaultLighting.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.InputElementsLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float3, "a_Normal"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float3, "a_Tangent"},
			{ShaderDataType::Float3, "a_Bitangent"},
		};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(descriptorLayout);
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(Renderer::GetMaterialDescriptorSetLayout());
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
		const auto& framebufferTextureColor = FramebufferTexture2D::Create(device, FramebufferTextureFormat::Color, width, height);
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { framebufferTextureColor, s_DepthbufferTexture };
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Color }, { FramebufferTextureFormat::Depth } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& defaultRenderSubsystem = CreateRef<DefaultRenderSubsystem>(pipeline);
		defaultRenderSubsystem->SetDescriptorChunk({ descriptorSet });
		Renderer::AddRenderSubsystem(defaultRenderSubsystem);
		Renderer::SetScreenTexture(framebufferTextureColor);
	}

	void SubsystemManager::InitDefaultGBufferSubsystem()
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

		const auto& shader = Shader::Create("assets/shaders/GBuffer.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.InputElementsLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float3, "a_Normal"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float3, "a_Tangent"},
			{ShaderDataType::Float3, "a_Bitangent"},
		};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(descriptorLayout);
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
		s_PositionsViewTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::RGBA16F, width, height);
		s_PositionsWorldTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::RGBA16F, width, height);
		s_AlbedoTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::RGBA8, width, height);
		s_NormalMetalRoughnessTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::RGBA16F, width, height);

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
		gBufferRenderSubsystem->SetDescriptorChunk({ descriptorSet });
		Renderer::AddRenderSubsystem(gBufferRenderSubsystem);
	}

	void SubsystemManager::InitDefaultDeferredSubsystem()
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

		// Geometry Layout
		DescriptorBinding positionsView{ DescriptorType::ImageSampler, 0 };
		DescriptorBinding positionsWorld{ DescriptorType::ImageSampler, 1 };
		DescriptorBinding albedo{ DescriptorType::ImageSampler, 2 };
		DescriptorBinding normMetRough{ DescriptorType::ImageSampler, 3 };
		DescriptorSetLayoutSpecification geomLayout{};
		geomLayout.Bindings = { positionsView, positionsWorld, albedo, normMetRough };

		Ref<DescriptorSetLayout> sceneDescriptorLayout = DescriptorSetLayout::Create(sceneLayout, device);
		Ref<DescriptorSetLayout> geomDescriptorLayout = DescriptorSetLayout::Create(geomLayout, device);
		Ref<DescriptorSet> sceneSet = DescriptorSet::Create(device, sceneDescriptorLayout, Renderer::GetDescriptorPool());
		Ref<DescriptorSet> geomSet = DescriptorSet::Create(device, geomDescriptorLayout, Renderer::GetDescriptorPool());

		sceneSet->WriteUniformBuffer(s_CameraUniformBuffer, (u32)sizeof(GlobalUBO));
		sceneSet->WriteUniformBuffer(s_FogUniformBuffer, (u32)sizeof(FogUBO));
		sceneSet->WriteUniformBuffer(s_PhysicalCameraUniformBuffer, (u32)sizeof(PhysicalCameraUBO));
		sceneSet->WriteUniformBuffer(s_LightsUniformBuffer, (u32)sizeof(LightsUBO));
		sceneSet->WriteTexture2D(s_PreintegratedFG);
		sceneSet->WriteTextureCube(s_Skybox);

		geomSet->WriteFramebufferTexture2D(s_PositionsViewTexture);
		s_PositionsWorldTexture->SetBinding(1);
		geomSet->WriteFramebufferTexture2D(s_PositionsWorldTexture);
		s_AlbedoTexture->SetBinding(2);
		geomSet->WriteFramebufferTexture2D(s_AlbedoTexture);
		s_NormalMetalRoughnessTexture->SetBinding(3);
		geomSet->WriteFramebufferTexture2D(s_NormalMetalRoughnessTexture);

		const auto& shader = Shader::Create("assets/shaders/DefaultDeferred.glsl");
		PipelineSpecification pipelineSpec{};
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(sceneDescriptorLayout);
		pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(geomDescriptorLayout);
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
		const auto& framebufferTextureColor = FramebufferTexture2D::Create(device, FramebufferTextureFormat::Color, width, height);
		FramebufferSpecification framebufferSpec{};
		framebufferSpec.Width = width;
		framebufferSpec.Height = height;
		framebufferSpec.RenderTargets = { framebufferTextureColor, s_DepthbufferTexture };
		framebufferSpec.TextureSpecs = { { FramebufferTextureFormat::Color }, { FramebufferTextureFormat::Depth } };

		const auto& framebuffer = Framebuffer::Create(framebufferSpec, device, renderPass);
		renderPass->SetFramebuffer(framebuffer);

		const auto& defaultDeferredSubsystem = CreateRef<DrawToBufferRenderSubsystem>(pipeline);
		defaultDeferredSubsystem->SetDescriptorChunk({ sceneSet, geomSet });
		Renderer::AddRenderSubsystem(defaultDeferredSubsystem);
		Renderer::SetScreenTexture(framebufferTextureColor);
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
	}

	void SubsystemManager::InitTextures()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

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
}