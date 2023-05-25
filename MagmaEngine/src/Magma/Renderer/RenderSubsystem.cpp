#include "mgmpch.h"
#include "RenderSubsystem.h"

namespace Magma
{
	RenderSubsystem::RenderSubsystem(const Ref<Pipeline>& pipeline)
		: m_Pipeline(pipeline)
	{
	}

	RenderSubsystem::~RenderSubsystem()
	{
		for (auto& obj : m_GameObjects)
			obj = nullptr;
		m_Pipeline = nullptr;
	}

	void RenderSubsystem::Bind()
	{
		RenderCommand::BindPipeline(m_Pipeline);
		m_DescriptorChunk.Bind(m_Pipeline);
	}

	bool RenderSubsystem::TryAddGameObject(const Ref<GameObject>& gameObject)
	{
		bool canBeAdded = IsGameObjectSuitable(gameObject);
		if (canBeAdded) m_GameObjects.push_back(gameObject);
		return canBeAdded;
	}

	void DescriptorChunk::Bind(const Ref<Pipeline>& pipeline)
	{
		for (u32 i = 0; i < m_DescriptorSets.size(); i++)
			RenderCommand::BindDescriptorSet(m_DescriptorSets[i], pipeline, i);
	}
}