#include "mgmpch.h"
#include "DrawToBufferRenderSubsystem.h"

namespace Magma
{
	DrawToBufferRenderSubsystem::DrawToBufferRenderSubsystem(const Ref<Pipeline>& pipeline)
		: RenderSubsystem(pipeline)
	{
	}

	DrawToBufferRenderSubsystem::~DrawToBufferRenderSubsystem()
	{
		m_Pipeline = nullptr;
	}

	bool DrawToBufferRenderSubsystem::IsGameObjectSuitable(const Ref<GameObject>& gameObject)
	{
		return true;
	}

	void DrawToBufferRenderSubsystem::RenderGameObjects()
	{
		RenderCommand::DrawVertices(6, 1, 0, 0);
	}
}