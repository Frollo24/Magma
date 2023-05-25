#include "mgmpch.h"
#include "SubsystemManager.h"

#include "Magma/Core/Application.h"
#include "Renderer.h"
#include "RenderSwapchain.h"
#include "Descriptors.h"

#include "Subsystems/DepthPrepassRenderSubsystem.h"
#include "Subsystems/SimpleRenderSubsystem.h"
#include "Subsystems/DefaultRenderSubsystem.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Magma
{
	// TEMPORARY
	static Ref<UniformBuffer> s_SimpleCameraUniformBuffer = nullptr;
	static Ref<UniformBuffer> s_DirLightUniformBuffer = nullptr;

	static Ref<FramebufferTexture2D> s_DepthbufferTexture = nullptr;

	static Ref<UniformBuffer> s_CameraUniformBuffer = nullptr; // Camera
	static Ref<UniformBuffer> s_FogUniformBuffer = nullptr; // Fog
	static Ref<UniformBuffer> s_PhysicalCameraUniformBuffer = nullptr; // PhysicalCamera
	static Ref<UniformBuffer> s_LightsUniformBuffer = nullptr; // Lights

	void SubsystemManager::InitSubsystems()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		s_DepthbufferTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::Depth, width, height);

		// TODO: enable subsystem selection
		InitDepthPrepassSubsystem();
		// InitSimpleSubsystem();
		InitDefaultSubsystem();
	}

	void SubsystemManager::Shutdown()
	{
		s_CameraUniformBuffer = nullptr;
		s_FogUniformBuffer = nullptr;
		s_PhysicalCameraUniformBuffer = nullptr;
		s_LightsUniformBuffer = nullptr;

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

	void SubsystemManager::InitDefaultSubsystem()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		// Scene Layout
		DescriptorBinding cameraTransforms{ DescriptorType::UniformBuffer, 0 };
		DescriptorBinding fogSettings{ DescriptorType::UniformBuffer, 1 };
		DescriptorBinding physicalCamera{ DescriptorType::UniformBuffer, 2 };
		DescriptorBinding lights{ DescriptorType::UniformBuffer, 3 };
		DescriptorSetLayoutSpecification sceneLayout{};
		sceneLayout.Bindings = { cameraTransforms, fogSettings, physicalCamera, lights };

		Ref<DescriptorSetLayout> descriptorLayout = DescriptorSetLayout::Create(sceneLayout, device);
		Ref<DescriptorSet> descriptorSet = DescriptorSet::Create(device, descriptorLayout, Renderer::GetDescriptorPool());

		s_CameraUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(GlobalUBO), 0, 2);
		s_FogUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(FogUBO), 1, 2);
		s_PhysicalCameraUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(PhysicalCameraUBO), 2, 2);
		s_LightsUniformBuffer = UniformBuffer::Create(device, (u32)sizeof(LightsUBO), 3, 2);

		descriptorSet->WriteUniformBuffer(s_CameraUniformBuffer, (u32)sizeof(GlobalUBO));
		descriptorSet->WriteUniformBuffer(s_FogUniformBuffer, (u32)sizeof(FogUBO));
		descriptorSet->WriteUniformBuffer(s_PhysicalCameraUniformBuffer, (u32)sizeof(PhysicalCameraUBO));
		descriptorSet->WriteUniformBuffer(s_LightsUniformBuffer, (u32)sizeof(LightsUBO));

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
		fog.fogColor = glm::vec4(0.01f, 0.01f, 0.01f, 0.07f);
		s_FogUniformBuffer->SetCommonDataForAllFrames(&fog, sizeof(fog));

		LightsUBO lightsUbo{};
		DirLight dirLight;
		dirLight.color = { 1.0f, 0.99f, 0.96f, 1.0f};
		dirLight.direction = glm::vec3(-1.0f);
		dirLight.intensity = 10.0f;
		lightsUbo.dirLight[0] = dirLight;
		s_LightsUniformBuffer->SetCommonDataForAllFrames(&lightsUbo, sizeof(lightsUbo));

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
}