#include "mgmpch.h"
#include "MeshRenderer.h"

namespace Magma
{
	MeshRenderer::MeshRenderer(const Ref<Model>& model)
		: m_Model(model)
	{
	}

	MeshRenderer::~MeshRenderer()
	{
		m_Model = nullptr;
	}

	void MeshRenderer::Render()
	{
		m_Model->Render();
	}
}