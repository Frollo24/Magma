#include "mgmpch.h"
#include "SkyboxRenderSubsystem.h"

#include "Magma/Renderer/Renderer.h"
#include "Magma/Renderer/RenderCommand.h"

namespace Magma
{
	SkyboxRenderSubsystem::SkyboxRenderSubsystem(const Ref<Pipeline>& pipeline)
		: RenderSubsystem(pipeline)
	{
		m_SkyboxBuffer = VertexBuffer::Create(Renderer::GetDevice(), sizeof(c_SkyboxVertices), c_SkyboxVertices);
	}

	SkyboxRenderSubsystem::~SkyboxRenderSubsystem()
	{
		m_SkyboxBuffer = nullptr;
	}

	bool SkyboxRenderSubsystem::IsGameObjectSuitable(const Ref<GameObject>& gameObject)
	{
		return true;
	}

	void SkyboxRenderSubsystem::RenderGameObjects()
	{
		RenderCommand::BindVertexBuffer(m_SkyboxBuffer);
		RenderCommand::DrawVertices(36, 1, 0, 0);
	}
}