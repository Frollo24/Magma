#pragma once
#include "Magma/Renderer/RenderSubsystem.h"

#include "SSAOConstantData.h"

namespace Magma
{
	class SSAORenderSubsystem : public RenderSubsystem
	{
	public:
		SSAORenderSubsystem(const Ref<Pipeline>& pipeline);
		virtual ~SSAORenderSubsystem();

		virtual bool IsGameObjectSuitable(const Ref<GameObject>& gameObject) override;
		virtual void RenderGameObjects() override;

		static void CalculateKernelPositions(SSAOConstantData& ssaoData);
	};
}

