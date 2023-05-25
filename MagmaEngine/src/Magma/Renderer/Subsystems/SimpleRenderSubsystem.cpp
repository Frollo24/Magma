#include "mgmpch.h"
#include "SimpleRenderSubsystem.h"

namespace Magma
{
	SimpleRenderSubsystem::SimpleRenderSubsystem(const Ref<Pipeline>& pipeline)
		: RenderSubsystem(pipeline)
	{
	}

	SimpleRenderSubsystem::~SimpleRenderSubsystem()
	{
		m_Pipeline = nullptr;
	}

	bool SimpleRenderSubsystem::IsGameObjectSuitable(const Ref<GameObject>& gameObject)
	{
		return true;
	}

	void SimpleRenderSubsystem::RenderGameObjects()
	{
		for (const auto& gameObject : m_GameObjects)
		{
			auto& transform = gameObject->GetTransform()->GetMatrix();

			SimpleConstantData data{};
			data.modelMatrix = transform;
			data.tintColor = gameObject->GetMeshRenderer()->GetMaterial()->GetColor();

			RenderCommand::UploadConstantData(m_Pipeline, sizeof(SimpleConstantData), &data);
			gameObject->Render();
		}
	}
}