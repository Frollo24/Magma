#pragma once
#include "Magma/Renderer/RenderSubsystem.h"

#include "SimpleConstantData.h"

namespace Magma
{
	class SimpleRenderSubsystem : public RenderSubsystem
	{
	public:
		SimpleRenderSubsystem(const Ref<Pipeline>& pipeline);
		virtual ~SimpleRenderSubsystem();

		virtual bool IsGameObjectSuitable(const Ref<GameObject>& gameObject) override;
		virtual void RenderGameObjects() override;
	};
}
