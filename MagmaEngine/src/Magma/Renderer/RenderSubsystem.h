#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/Pipeline.h"
#include "Magma/Renderer/RenderCommand.h"
#include "Magma/Elements/GameObject.h"

namespace Magma
{
	struct DescriptorChunk
	{
		DescriptorChunk(const std::initializer_list<Ref<DescriptorSet>>& descriptorSets)
			: m_DescriptorSets(descriptorSets)
		{
		}

		DescriptorChunk(const std::vector<Ref<DescriptorSet>>& descriptorSets = {})
			: m_DescriptorSets(descriptorSets)
		{
		}

		inline void PushDescriptorSet(const Ref<DescriptorSet>& descriptorSet) { m_DescriptorSets.push_back(descriptorSet); }
		void Bind(const Ref<Pipeline>& pipeline);

	private:
		std::vector<Ref<DescriptorSet>> m_DescriptorSets{};
	};

	class MAGMA_API RenderSubsystem
	{
	public:
		RenderSubsystem(const Ref<Pipeline>& pipeline);
		virtual ~RenderSubsystem();

		inline const Ref<Pipeline>& GetPipeline() const { return m_Pipeline; }
		inline const DescriptorChunk& GetDescriptorChunk() const { return m_DescriptorChunk; }
		inline void SetDescriptorChunk(const DescriptorChunk& descriptorChunk) { m_DescriptorChunk = descriptorChunk; }

		void Bind();

		virtual bool TryAddGameObject(const Ref<GameObject>& gameObject);
		virtual bool IsGameObjectSuitable(const Ref<GameObject>& gameObject) = 0;
		virtual void RenderGameObjects() = 0;

	protected:
		Ref<Pipeline> m_Pipeline;
		DescriptorChunk m_DescriptorChunk{};
		std::vector<Ref<GameObject>> m_GameObjects{};

		u32 m_Frame = 0;
	};
}

