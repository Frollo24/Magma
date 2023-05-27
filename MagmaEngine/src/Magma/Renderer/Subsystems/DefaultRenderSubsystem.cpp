#include "mgmpch.h"
#include "DefaultRenderSubsystem.h"

namespace Magma
{
	DefaultRenderSubsystem::DefaultRenderSubsystem(const Ref<Pipeline>& pipeline)
		: RenderSubsystem(pipeline)
	{
	}

	DefaultRenderSubsystem::~DefaultRenderSubsystem()
	{
		for (const auto& [material, textureArray] : m_MaterialTexturesRef)
			m_MaterialTexturesRef[material] = nullptr;

		m_MaterialTexturesRef.clear();
	}

	bool DefaultRenderSubsystem::IsGameObjectSuitable(const Ref<GameObject>& gameObject)
	{
		if (!gameObject->GetMeshRenderer()) return false;

		auto& material = gameObject->GetMeshRenderer()->GetMaterial();
		if (!m_EnableNonPBR && !material->GetIsPBR()) return false;

		if (m_MaterialTexturesRef.empty())
			m_DescriptorChunk.PushDescriptorSet(material->GetDescriptorSet());

		m_MaterialTexturesRef[material] = material->GetDescriptorSet();
		m_MaterialGameObjects[material].push_back(gameObject);

		return true;
	}

	void DefaultRenderSubsystem::RenderGameObjects()
	{
		for (const auto& [material, textureArray] : m_MaterialTexturesRef)
		{
			RenderCommand::BindDescriptorSet(textureArray, m_Pipeline, 1);
			for (const auto& gameObject : m_MaterialGameObjects[material])
			{
				auto& transform = gameObject->GetTransform()->GetMatrix();

				DefaultConstantData data{};
				data.modelMatrix = transform;
				data.tintColor = material->GetColor();
				data.metallic = material->GetMetallic();
				data.roughness = material->GetRoughness();
				data.isPBR = material->GetIsPBR() ? 1.0f : 0.0f;

				RenderCommand::UploadConstantData(m_Pipeline, sizeof(DefaultConstantData), &data);
				gameObject->Render();
			}
		}
	}
}