#include "SandboxLayer.h"

// TEMPORARY
static Magma::Ref<Magma::Shader> s_Shader = nullptr;
static Magma::Ref<Magma::Pipeline> s_Pipeline = nullptr;

SandboxLayer::SandboxLayer() : Layer("Sandbox Layer")
{
	s_Shader = Magma::Shader::Create("assets/shaders/VulkanTestNoBuffers.glsl");
	Magma::PipelineSpecification spec{};
	spec.Shader = s_Shader;

	const auto& instance = Magma::Application::Instance().GetWindow().GetGraphicsInstance();
	const auto& device = instance->GetDevice();
	const auto& renderPass = instance->GetSwapchain()->GetMainRenderPass();
	s_Pipeline = Magma::Pipeline::Create(spec, device, renderPass);
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
	Magma::RenderCommand::DrawVertices(3, 1, 0, 0);
	Magma::RenderCommand::EndRenderPass(renderPass);
}

void SandboxLayer::OnEvent(Magma::Event& event)
{
	MGM_TRACE("{0}", event);
}
