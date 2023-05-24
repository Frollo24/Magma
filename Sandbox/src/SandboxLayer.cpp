#include "SandboxLayer.h"

// TEMPORARY
#include <glm/gtc/matrix_transform.hpp>

static Magma::Ref<Magma::Shader> s_Shader = nullptr;
static Magma::Ref<Magma::Pipeline> s_Pipeline = nullptr;
static Magma::Ref<Magma::Model> s_Model = nullptr;
static glm::mat4 s_ViewProj;

static Magma::Ref<Magma::DescriptorSetLayout> s_DescriptorLayout = nullptr;
static Magma::Ref<Magma::DescriptorPool> s_DescriptorPool = nullptr;
static Magma::Ref<Magma::DescriptorSet> s_DescriptorSet = nullptr;
static Magma::Ref<Magma::UniformBuffer> s_UniformBuffer = nullptr;
static Magma::Ref<Magma::Texture2D> s_Texture = nullptr;

static Magma::Ref<Magma::RenderPass> s_RenderPass = nullptr;
static Magma::Ref<Magma::Framebuffer> s_Framebuffer = nullptr;
static Magma::Ref<Magma::FramebufferTexture2D> s_FramebufferTextureColor = nullptr;
static Magma::Ref<Magma::FramebufferTexture2D> s_FramebufferTextureDepth = nullptr;

struct TestConstantData
{
	// glm::mat4 viewProj;
	glm::vec4 tintColor;
	glm::vec3 posOffset;
};

SandboxLayer::SandboxLayer() : Layer("Sandbox Layer")
{
	const auto& instance = Magma::Application::Instance().GetWindow().GetGraphicsInstance();
	const auto& device = instance->GetDevice();

	Magma::DescriptorBinding matrixTransform{ Magma::DescriptorType::UniformBuffer, 0 };
	Magma::DescriptorBinding texture2D{ Magma::DescriptorType::ImageSampler, 1 };
	Magma::DescriptorSetLayoutSpecification layoutSpec{};
	layoutSpec.Bindings = { matrixTransform, texture2D };
	s_DescriptorLayout = Magma::DescriptorSetLayout::Create(layoutSpec, device);
	s_DescriptorPool = Magma::DescriptorPool::Create(device);
	s_DescriptorSet = Magma::DescriptorSet::Create(device, s_DescriptorLayout, s_DescriptorPool);

	s_UniformBuffer = Magma::UniformBuffer::Create(device, sizeof(glm::mat4), 0, 2);
	s_DescriptorSet->WriteUniformBuffer(s_UniformBuffer, sizeof(glm::mat4));

	bool generateMipmapsOnLoad = false;
	s_Texture = Magma::Texture2D::Create(device, "assets/textures/texture-wood.jpg", generateMipmapsOnLoad);
	s_Texture->GenerateMipmaps();
	s_Texture->SetBinding(1);
	s_DescriptorSet->WriteTexture2D(s_Texture);

	s_Shader = Magma::Shader::Create("assets/shaders/VulkanTestDescriptors.glsl");
	Magma::PipelineSpecification pipelineSpec{};
	pipelineSpec.InputElementsLayout = {
		{Magma::ShaderDataType::Float3, "a_Position"},
		{Magma::ShaderDataType::Float3, "a_Normal"},
		{Magma::ShaderDataType::Float2, "a_TexCoord"},
		{Magma::ShaderDataType::Float3, "a_Tangent"},
		{Magma::ShaderDataType::Float3, "a_Bitangent"},
	};
	pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_DescriptorLayout);
	pipelineSpec.GlobalDataLayout.ConstantDataSize = sizeof(TestConstantData);
	pipelineSpec.Shader = s_Shader;

	Magma::RenderPassSpecification renderPassSpec{};
	renderPassSpec.ClearValues.ClearFlags |= Magma::ClearFlags::Color | Magma::ClearFlags::DepthStencil;
	renderPassSpec.ClearValues.Color = { 0.1f, 0.1f, 0.1f, 1.0f };
	renderPassSpec.Attachments = { Magma::AttachmentFormat::RGBA8, Magma::AttachmentFormat::D24S8 };
	renderPassSpec.IsSwapchainTarget = false;

	s_RenderPass = Magma::RenderPass::Create(renderPassSpec, device);
	s_Pipeline = Magma::Pipeline::Create(pipelineSpec, device, s_RenderPass);

	s_Model = Magma::CreateRef<Magma::Model>("assets/models/UVSphere.obj", device);

	s_ViewProj = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 25.0f)
		* glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	s_UniformBuffer->SetCommonDataForAllFrames(&s_ViewProj, sizeof(glm::mat4));

	Magma::u32 width = instance->GetSwapchain()->GetWidth();
	Magma::u32 height = instance->GetSwapchain()->GetHeight();
	s_FramebufferTextureColor = Magma::FramebufferTexture2D::Create(device, Magma::FramebufferTextureFormat::Color, width, height);
	s_FramebufferTextureDepth = Magma::FramebufferTexture2D::Create(device, Magma::FramebufferTextureFormat::Depth, width, height);
	Magma::FramebufferSpecification framebufferSpec{};
	framebufferSpec.Width = width;
	framebufferSpec.Height = height;
	framebufferSpec.RenderTargets = { s_FramebufferTextureColor, s_FramebufferTextureDepth };
	framebufferSpec.TextureSpecs = { { Magma::FramebufferTextureFormat::Color }, { Magma::FramebufferTextureFormat::Depth } };

	s_Framebuffer = Magma::Framebuffer::Create(framebufferSpec, device, s_RenderPass);
	s_RenderPass->SetFramebuffer(s_Framebuffer);

	Magma::Renderer::SetScreenTexture(s_FramebufferTextureColor);
}

SandboxLayer::~SandboxLayer()
{
	s_FramebufferTextureDepth = nullptr;
	s_FramebufferTextureColor = nullptr;
	s_Framebuffer = nullptr;
	s_RenderPass = nullptr;

	s_Texture = nullptr;
	s_UniformBuffer = nullptr;
	s_DescriptorSet = nullptr;
	s_DescriptorPool = nullptr;
	s_DescriptorLayout = nullptr;

	s_Model = nullptr;
	s_Pipeline = nullptr;
	s_Shader = nullptr;
}

void SandboxLayer::OnUpdate()
{
	// TEMPORARY
	const auto& window = Magma::Application::Instance().GetWindow();

	TestConstantData data{};
	// data.viewProj = s_ViewProj;

	auto& lookAt = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	auto& rotate = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f * (float)Magma::Time::TotalTime), glm::vec3(1.0f, 0.0f, 0.0f));
	s_ViewProj = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 25.0f) * rotate * lookAt;
	s_UniformBuffer->SetData(&s_ViewProj, sizeof(glm::mat4));

	Magma::RenderCommand::BeginRenderPass(s_RenderPass);
	Magma::RenderCommand::SetViewport(0, 0, window.GetWidth(), window.GetHeight());
	Magma::RenderCommand::SetScissor(0, 0, window.GetWidth(), window.GetHeight());
	Magma::RenderCommand::BindPipeline(s_Pipeline);
	Magma::RenderCommand::BindDescriptorSet(s_DescriptorSet, s_Pipeline, 0);
	{
		data.tintColor = glm::vec4(0.5f, 0.3f, 0.2f, 1.0f);
		data.posOffset = glm::vec4(0.5f);
		Magma::RenderCommand::UploadConstantData(s_Pipeline, sizeof(TestConstantData), &data);
		s_Model->Render();
	}
	{
		data.tintColor = glm::vec4(1.0f);
		data.posOffset = glm::vec4(-0.5f);
		Magma::RenderCommand::UploadConstantData(s_Pipeline, sizeof(TestConstantData), &data);
		s_Model->Render();
	}
	{
		data.tintColor = glm::vec4(0.4f, 1.0f, 0.3f, 1.0f);
		data.posOffset = glm::vec4(1.5f);
		Magma::RenderCommand::UploadConstantData(s_Pipeline, sizeof(TestConstantData), &data);
		s_Model->Render();
	}
	
	Magma::RenderCommand::EndRenderPass(s_RenderPass);
	Magma::Renderer::DrawToScreen();
}

void SandboxLayer::OnEvent(Magma::Event& event)
{
	MGM_TRACE("{0}", event);
}
