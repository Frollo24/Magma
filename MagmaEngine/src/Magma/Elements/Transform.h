#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Elements/Model.h"

namespace Magma
{
	class MAGMA_API Transform
	{
	public:
		Transform(const glm::mat4& transform = glm::mat4(1.0f));
		virtual ~Transform() = default;

		inline const glm::mat4& GetMatrix() const { return m_Transform; }

	private:
		glm::mat4 m_Transform;
	};
}