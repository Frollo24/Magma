#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Elements/Transform.h"
#include "Magma/Elements/MeshRenderer.h"
#include "Magma/Elements/Material.h"

namespace Magma
{
	class MAGMA_API GameObject
	{
	public:
		GameObject(const Ref<Transform>& transform = CreateRef<Transform>());
		virtual ~GameObject();

		void Render();

		inline const Ref<Transform>& GetTransform() { return m_Transform; }
		inline void SetTransform(const Ref<Transform>& transform) { m_Transform = transform; }

		inline const Ref<MeshRenderer>& GetMeshRenderer() { return m_MeshRenderer; }
		inline void SetMeshRenderer(const Ref<MeshRenderer>& meshRenderer) { m_MeshRenderer = meshRenderer; }

	private:
		Ref<Transform> m_Transform = nullptr;
		Ref<MeshRenderer> m_MeshRenderer = nullptr;
	};
}
