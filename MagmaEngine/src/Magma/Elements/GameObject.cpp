#include "mgmpch.h"
#include "GameObject.h"

namespace Magma
{
	GameObject::GameObject(const Ref<Transform>& transform)
		: m_Transform(transform)
	{
	}

	GameObject::~GameObject()
	{
		m_Transform = nullptr;
		m_MeshRenderer = nullptr;
	}

	void GameObject::Render()
	{
		m_MeshRenderer->Render();
	}
}
