#pragma once
#include "Magma/Renderer/RenderSubsystem.h"

#include "DepthPrepassConstantData.h"

namespace Magma
{
	class DepthPrepassRenderSubsystem : public RenderSubsystem
	{
	public:
		DepthPrepassRenderSubsystem(const Ref<Pipeline>& pipeline);
		virtual ~DepthPrepassRenderSubsystem();

		virtual bool IsGameObjectSuitable(const Ref<GameObject>& gameObject) override;
		virtual void RenderGameObjects() override;
	};
}

