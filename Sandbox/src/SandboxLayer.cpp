#include "SandboxLayer.h"

// TEMPORARY
static Magma::Ref<Magma::Shader> s_Shader = nullptr;
static Magma::Ref<Magma::Pipeline> s_Pipeline = nullptr;
static Magma::Ref<Magma::VertexBuffer> s_VertexBuffer = nullptr;
static Magma::Ref<Magma::IndexBuffer> s_IndexBuffer = nullptr;

SandboxLayer::SandboxLayer() : Layer("Sandbox Layer")
{
	s_Shader = Magma::Shader::Create("assets/shaders/VulkanTest.glsl");
	Magma::PipelineSpecification spec{};
	spec.InputElementsLayout = {
		{Magma::ShaderDataType::Float3, "a_Position"},
		{Magma::ShaderDataType::Float3, "a_Color"},
	};
	spec.Shader = s_Shader;

	const auto& instance = Magma::Application::Instance().GetWindow().GetGraphicsInstance();
	const auto& device = instance->GetDevice();
	const auto& renderPass = instance->GetSwapchain()->GetMainRenderPass();
	s_Pipeline = Magma::Pipeline::Create(spec, device, renderPass);

	float squareVertices[6 * 4] = {
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
	};
	uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

	s_VertexBuffer = Magma::VertexBuffer::Create(device, sizeof(squareVertices), squareVertices);
	s_IndexBuffer = Magma::IndexBuffer::Create(device, sizeof(squareIndices), squareIndices);
}

SandboxLayer::~SandboxLayer()
{
	s_VertexBuffer = nullptr;
	s_IndexBuffer = nullptr;
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
	Magma::RenderCommand::BindVertexBuffer(s_VertexBuffer);
	Magma::RenderCommand::BindIndexBuffer(s_IndexBuffer);
	Magma::RenderCommand::DrawIndices(6, 1, 0, 0);
	Magma::RenderCommand::EndRenderPass(renderPass);
}

void SandboxLayer::OnEvent(Magma::Event& event)
{
	MGM_TRACE("{0}", event);
}
