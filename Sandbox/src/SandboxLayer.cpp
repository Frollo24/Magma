#include "SandboxLayer.h"

// TEMPORARY
static Magma::Ref<Magma::Shader> s_Shader = nullptr;
static Magma::Ref<Magma::Pipeline> s_Pipeline = nullptr;
static Magma::Ref<Magma::Model> s_Model = nullptr;

SandboxLayer::SandboxLayer() : Layer("Sandbox Layer")
{
	s_Shader = Magma::Shader::Create("assets/shaders/VulkanTestModel.glsl");
	Magma::PipelineSpecification spec{};
	spec.InputElementsLayout = {
		{Magma::ShaderDataType::Float3, "a_Position"},
		{Magma::ShaderDataType::Float3, "a_Normal"},
		{Magma::ShaderDataType::Float2, "a_TexCoord"},
		{Magma::ShaderDataType::Float3, "a_Tangent"},
		{Magma::ShaderDataType::Float3, "a_Bitangent"},
	};
	spec.Shader = s_Shader;

	const auto& instance = Magma::Application::Instance().GetWindow().GetGraphicsInstance();
	const auto& device = instance->GetDevice();
	const auto& renderPass = instance->GetSwapchain()->GetMainRenderPass();
	s_Pipeline = Magma::Pipeline::Create(spec, device, renderPass);

	s_Model = Magma::CreateRef<Magma::Model>("assets/models/UVSphere.obj", device);
}

SandboxLayer::~SandboxLayer()
{
	s_Model = nullptr;
	s_Pipeline = nullptr;
	s_Shader = nullptr;
}

void SandboxLayer::OnUpdate()
{
	// TEMPORARY
	const auto& window = Magma::Application::Instance().GetWindow();
	const auto& renderPass = window.GetGraphicsInstance()->GetSwapchain()->GetMainRenderPass();

	Magma::RenderCommand::BeginRenderPass(renderPass);
	Magma::RenderCommand::SetViewport(0, 0, window.GetWidth(), window.GetHeight());
	Magma::RenderCommand::SetScissor(0, 0, window.GetWidth(), window.GetHeight());
	Magma::RenderCommand::BindPipeline(s_Pipeline);
	s_Model->Render();
	Magma::RenderCommand::EndRenderPass(renderPass);
}

void SandboxLayer::OnEvent(Magma::Event& event)
{
	MGM_TRACE("{0}", event);
}
