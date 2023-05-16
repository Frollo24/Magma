#pragma once
#include <Magma.h>


class SandboxLayer : public Magma::Layer
{
public:
	SandboxLayer();
	virtual ~SandboxLayer();

	void OnUpdate() override;
	void OnEvent(Magma::Event& event) override;
};

