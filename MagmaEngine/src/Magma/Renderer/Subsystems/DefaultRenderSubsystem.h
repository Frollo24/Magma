#pragma once
#include "Magma/Renderer/RenderSubsystem.h"

#include "DefaultConstantData.h"

namespace Magma
{
	class DefaultRenderSubsystem : public RenderSubsystem
	{
	public:
		DefaultRenderSubsystem(const Ref<Pipeline>& pipeline);
		virtual ~DefaultRenderSubsystem();

		virtual bool IsGameObjectSuitable(const Ref<GameObject>& gameObject) override;
		virtual void RenderGameObjects() override;

		inline void EnableNonPBRRendering(const bool& enable) { m_EnableNonPBR = enable; }

	private:
		std::unordered_map<Ref<Material>, Ref<DescriptorSet>> m_MaterialTexturesRef{};
		std::unordered_map<Ref<Material>, std::vector<Ref<GameObject>>> m_MaterialGameObjects{};

		bool m_EnableNonPBR = false;
	};
}

