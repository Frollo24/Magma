#include "SandboxLayer.h"

// TEMPORARY
#include <glm/gtc/matrix_transform.hpp>

static Magma::Ref<Magma::Shader> s_Shader = nullptr;
static Magma::Ref<Magma::Pipeline> s_Pipeline = nullptr;
static Magma::Ref<Magma::Model> s_Model = nullptr;

static Magma::Ref<Magma::DescriptorSetLayout> s_DescriptorLayout = nullptr;
static Magma::Ref<Magma::DescriptorPool> s_DescriptorPool = nullptr;
static Magma::Ref<Magma::DescriptorSet> s_DescriptorSet = nullptr;
static Magma::Ref<Magma::UniformBuffer> s_CameraUBO = nullptr;
static Magma::Ref<Magma::UniformBuffer> s_LightUBO = nullptr;

static Magma::Ref<Magma::RenderPass> s_RenderPass = nullptr;
static Magma::Ref<Magma::Framebuffer> s_Framebuffer = nullptr;
static Magma::Ref<Magma::FramebufferTexture2D> s_FramebufferTextureColor = nullptr;
static Magma::Ref<Magma::FramebufferTexture2D> s_FramebufferTextureDepth = nullptr;

struct CameraUBO
{
	glm::mat4 viewProj;
	glm::mat4 proj;
	glm::mat4 view;
};
static CameraUBO s_CameraData{};

struct DirLight
{
	glm::vec4 color;
	glm::vec3 direction;
	float intensity;
};
static DirLight s_DirLightData{};

SandboxLayer::SandboxLayer() : Layer("Sandbox Layer")
{
	const auto& instance = Magma::Application::Instance().GetWindow().GetGraphicsInstance();
	const auto& device = instance->GetDevice();

	Magma::DescriptorBinding viewProj{ Magma::DescriptorType::UniformBuffer, 0 };
	Magma::DescriptorBinding dirLight{ Magma::DescriptorType::UniformBuffer, 1 };
	Magma::DescriptorSetLayoutSpecification layoutSpec{};
	layoutSpec.Bindings = { viewProj, dirLight };
	s_DescriptorLayout = Magma::DescriptorSetLayout::Create(layoutSpec, device);
	s_DescriptorPool = Magma::DescriptorPool::Create(device);
	s_DescriptorSet = Magma::DescriptorSet::Create(device, s_DescriptorLayout, s_DescriptorPool);

	s_CameraUBO = Magma::UniformBuffer::Create(device, sizeof(CameraUBO), 0, 2);
	s_LightUBO = Magma::UniformBuffer::Create(device, sizeof(DirLight), 1, 2);
	s_DescriptorSet->WriteUniformBuffer(s_CameraUBO, sizeof(CameraUBO));
	s_DescriptorSet->WriteUniformBuffer(s_LightUBO, sizeof(DirLight));

	s_CameraData.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	s_CameraData.proj = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 25.0f);
	s_CameraData.viewProj = s_CameraData.proj * s_CameraData.view;

	s_DirLightData.color = { 1.0f, 0.99f, 0.96f, 1.0f };
	s_DirLightData.direction = { -1.0f, -1.0f, -1.0f };
	s_DirLightData.intensity = 1.0f;

	s_Shader = Magma::Shader::Create("assets/shaders/BasicModelLighting.glsl");
	Magma::PipelineSpecification pipelineSpec{};
	pipelineSpec.InputElementsLayout = {
		{Magma::ShaderDataType::Float3, "a_Position"},
		{Magma::ShaderDataType::Float3, "a_Normal"},
		{Magma::ShaderDataType::Float2, "a_TexCoord"},
		{Magma::ShaderDataType::Float3, "a_Tangent"},
		{Magma::ShaderDataType::Float3, "a_Bitangent"},
	};
	pipelineSpec.GlobalDataLayout.DescriptorLayouts.push_back(s_DescriptorLayout);
	pipelineSpec.GlobalDataLayout.ConstantDataSize = sizeof(Magma::SimpleConstantData);
	pipelineSpec.Shader = s_Shader;

	Magma::RenderPassSpecification renderPassSpec{};
	renderPassSpec.ClearValues.ClearFlags |= Magma::ClearFlags::Color | Magma::ClearFlags::DepthStencil;
	renderPassSpec.ClearValues.Color = { 0.1f, 0.1f, 0.1f, 1.0f };
	renderPassSpec.Attachments = { Magma::AttachmentFormat::RGBA8, Magma::AttachmentFormat::D24S8 };
	renderPassSpec.IsSwapchainTarget = false;

	s_RenderPass = Magma::RenderPass::Create(renderPassSpec, device);
	s_Pipeline = Magma::Pipeline::Create(pipelineSpec, device, s_RenderPass);

	s_CameraUBO->SetCommonDataForAllFrames(&s_CameraData, sizeof(CameraUBO));
	s_LightUBO->SetCommonDataForAllFrames(&s_DirLightData, sizeof(DirLight));

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

	const auto& simpleRenderSubsystem = Magma::CreateRef<Magma::SimpleRenderSubsystem>(s_Pipeline);
	simpleRenderSubsystem->SetDescriptorChunk({s_DescriptorSet});
	Magma::Renderer::AddRenderSubsystem(simpleRenderSubsystem);
	Magma::Renderer::SetScreenTexture(s_FramebufferTextureColor);

	const auto& gameObject = Magma::CreateRef<Magma::GameObject>();
	const auto& meshRenderer = Magma::CreateRef<Magma::MeshRenderer>(Magma::CreateRef<Magma::Model>("assets/models/UVSphere.obj", device));
	meshRenderer->SetMaterial(Magma::CreateRef<Magma::Material>());
	gameObject->SetMeshRenderer(meshRenderer);
	Magma::Renderer::AddGameObject(gameObject);
}

SandboxLayer::~SandboxLayer()
{
	s_FramebufferTextureDepth = nullptr;
	s_FramebufferTextureColor = nullptr;
	s_Framebuffer = nullptr;
	s_RenderPass = nullptr;

	s_LightUBO = nullptr;
	s_CameraUBO = nullptr;
	s_DescriptorSet = nullptr;
	s_DescriptorPool = nullptr;
	s_DescriptorLayout = nullptr;

	s_Model = nullptr;
	s_Pipeline = nullptr;
	s_Shader = nullptr;
}

void SandboxLayer::OnUpdate()
{
	Magma::Renderer::RenderGameObjects();
	Magma::Renderer::DrawToScreen();
}

void SandboxLayer::OnEvent(Magma::Event& event)
{
	MGM_TRACE("{0}", event);
}
