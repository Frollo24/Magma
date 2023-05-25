#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Elements/Model.h"
#include "Magma/Elements/Material.h"

namespace Magma
{
	class MAGMA_API MeshRenderer
	{
	public:
		MeshRenderer(const Ref<Model>& model);
		virtual ~MeshRenderer();

		inline const Ref<Model>& GetModel() { return m_Model; }
		inline void SetModel(const Ref<Model>& model) { m_Model = model; }

		inline const Ref<Material>& GetMaterial() { return m_Material; }
		inline void SetMaterial(const Ref<Material>& material) { m_Material = material; }

		void Render();

	private:
		Ref<Model> m_Model = nullptr;
		Ref<Material> m_Material = nullptr;
	};
}