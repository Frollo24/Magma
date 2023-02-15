#define MAGMA_ENTRY_POINT
#include <Magma.h>

class Sandbox : public Magma::Application
{
public:
	Sandbox() = default;
	virtual ~Sandbox() = default;
};

Magma::Application* Magma::CreateApplication()
{
	return new Sandbox();
}