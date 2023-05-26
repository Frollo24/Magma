#pragma once
#include "Magma/Renderer/RenderSubsystem.h"

#include "SkyboxConstantData.h"

namespace Magma
{
	class SkyboxRenderSubsystem : public RenderSubsystem
	{
	public:
		SkyboxRenderSubsystem(const Ref<Pipeline>& pipeline);
		virtual ~SkyboxRenderSubsystem();

		virtual bool IsGameObjectSuitable(const Ref<GameObject>& gameObject) override;
		virtual void RenderGameObjects() override;

	private:
		Ref<VertexBuffer> m_SkyboxBuffer = nullptr;
	};
}