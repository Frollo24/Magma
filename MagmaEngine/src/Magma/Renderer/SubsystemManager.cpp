#include "mgmpch.h"
#include "SubsystemManager.h"

#include "Magma/Core/Application.h"
#include "Renderer.h"
#include "RenderSwapchain.h"
#include "Descriptors.h"

#include "Subsystems/DepthPrepassRenderSubsystem.h"
#include "Subsystems/SimpleRenderSubsystem.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Magma
{
	// TEMPORARY
	static Ref<UniformBuffer> s_CameraUniformBuffer = nullptr;
	static Ref<UniformBuffer> s_DirLightUniformBuffer = nullptr;

	static Ref<FramebufferTexture2D> s_DepthbufferTexture = nullptr;

	void SubsystemManager::InitSubsystems()
	{
		const auto& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const auto& device = instance->GetDevice();

		u32 width = instance->GetSwapchain()->GetWidth();
		u32 height = instance->GetSwapchain()->GetHeight();
		s_DepthbufferTexture = FramebufferTexture2D::Create(device, FramebufferTextureFormat::Depth, width, height);

		// TODO: enable subsystem selection
		InitDepthPrepassSubsystem();
		InitSimpleSubsystem();
		InitDefaultSubsystem();
	}

	void SubsystemManager::Shutdown()
	{
		s_DepthbufferTexture = nullptr;

		s_CameraUniformBuffer = nullptr;
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

		s_CameraUniformBuffer = UniformBuffer::Create(device, sizeof(SimpleCameraUBO), 0, 2);
		s_DirLightUniformBuffer = UniformBuffer::Create(device, sizeof(SimpleDirLightUBO), 1, 2);
		descriptorSet->WriteUniformBuffer(s_CameraUniformBuffer, sizeof(SimpleCameraUBO));
		descriptorSet->WriteUniformBuffer(s_DirLightUniformBuffer, sizeof(SimpleDirLightUBO));

		SimpleCameraUBO cameraData{};
		cameraData.view = Camera::Main->GetView();
		cameraData.proj = Camera::Main->GetProjection();
		cameraData.viewProj = Camera::Main->GetViewProjection();

		SimpleDirLightUBO dirLightData{};
		dirLightData.color = { 1.0f, 0.99f, 0.96f, 1.0f };
		dirLightData.direction = { -1.0f, -1.0f, -1.0f };
		dirLightData.intensity = 1.0f;

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

		s_CameraUniformBuffer->SetCommonDataForAllFrames(&cameraData, sizeof(SimpleCameraUBO));
		s_DirLightUniformBuffer->SetCommonDataForAllFrames(&dirLightData, sizeof(SimpleDirLightUBO));

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
		// TODO: add PBR shader
	}
}