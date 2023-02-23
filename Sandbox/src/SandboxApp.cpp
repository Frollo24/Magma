#define MAGMA_ENTRY_POINT
#include <Magma.h>

#include "SandboxLayer.h"

class Sandbox : public Magma::Application
{
public:
	Sandbox()
	{
		PushLayer(new SandboxLayer());
	}
	virtual ~Sandbox() = default;
};

Magma::Application* Magma::CreateApplication()
{
	return new Sandbox();
}