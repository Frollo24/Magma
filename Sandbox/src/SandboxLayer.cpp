#include "SandboxLayer.h"

SandboxLayer::SandboxLayer() : Layer("Sandbox Layer")
{
}

void SandboxLayer::OnUpdate()
{
	// MGM_INFO("SandboxLayer::OnUpdate");
}

void SandboxLayer::OnEvent(Magma::Event& event)
{
	MGM_TRACE("{0}", event);
}
