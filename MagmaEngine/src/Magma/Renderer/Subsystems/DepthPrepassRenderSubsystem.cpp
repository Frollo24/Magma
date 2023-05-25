#include "mgmpch.h"
#include "DepthPrepassRenderSubsystem.h"

#include "Magma/Renderer/Renderer.h"

namespace Magma
{
	DepthPrepassRenderSubsystem::DepthPrepassRenderSubsystem(const Ref<Pipeline>& pipeline)
		: RenderSubsystem(pipeline)
	{
	}

	DepthPrepassRenderSubsystem::~DepthPrepassRenderSubsystem()
	{
		m_Pipeline = nullptr;
	}

	bool DepthPrepassRenderSubsystem::IsGameObjectSuitable(const Ref<GameObject>& gameObject)
	{
		return true;
	}

	void DepthPrepassRenderSubsystem::RenderGameObjects()
	{
		for (const auto& gameObject : m_GameObjects)
		{
			auto& model = gameObject->GetTransform()->GetMatrix();
			auto& viewProj = Renderer::GetCamera()->GetViewProjection();

			DepthPrepassConstantData data{};
			data.modelViewProjMatrix = viewProj * model;

			RenderCommand::UploadConstantData(m_Pipeline, sizeof(DepthPrepassConstantData), &data);
			gameObject->Render();
		}
	}
}