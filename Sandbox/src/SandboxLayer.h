#pragma once
#include <Magma.h>


class SandboxLayer : public Magma::Layer
{
public:
	SandboxLayer();
	virtual ~SandboxLayer() = default;

	void OnUpdate() override;
	void OnEvent(Magma::Event& event) override;
};

