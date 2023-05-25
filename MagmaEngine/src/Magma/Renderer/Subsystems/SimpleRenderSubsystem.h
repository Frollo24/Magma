#pragma once
#include "Magma/Renderer/RenderSubsystem.h"

#include "SimpleConstantData.h"

namespace Magma
{
	// HACK: Specific render systems shouldn't be visible to the application
	class MAGMA_API SimpleRenderSubsystem : public RenderSubsystem
	{
	public:
		SimpleRenderSubsystem(const Ref<Pipeline>& pipeline);
		virtual ~SimpleRenderSubsystem();

		virtual bool IsGameObjectSuitable(const Ref<GameObject>& gameObject) override;
		virtual void RenderGameObjects() override;
	};
}

