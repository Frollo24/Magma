#include "SandboxLayer.h"

SandboxLayer::SandboxLayer() : Layer("Sandbox Layer")
{
}

void SandboxLayer::OnUpdate()
{
	// TEMPORARY
	const auto& renderPass = Magma::Application::Instance().GetWindow().GetGraphicsInstance()->GetSwapchain()->GetMainRenderPass();
	Magma::RenderCommand::BeginRenderPass(renderPass);
	Magma::RenderCommand::SetViewport(0, 0, 1600, 900);
	Magma::RenderCommand::SetScissor(0, 0, 1600, 900);
	Magma::RenderCommand::EndRenderPass(renderPass);
}

void SandboxLayer::OnEvent(Magma::Event& event)
{
	MGM_TRACE("{0}", event);
}
