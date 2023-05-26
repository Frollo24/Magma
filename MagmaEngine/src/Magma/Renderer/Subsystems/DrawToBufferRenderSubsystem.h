#pragma once
#include "Magma/Renderer/RenderSubsystem.h"

namespace Magma
{
	class DrawToBufferRenderSubsystem : public RenderSubsystem
	{
	public:
		DrawToBufferRenderSubsystem(const Ref<Pipeline>& pipeline);
		virtual ~DrawToBufferRenderSubsystem();
		
		virtual bool IsGameObjectSuitable(const Ref<GameObject>& gameObject) override;
		virtual void RenderGameObjects() override;
	};
}

