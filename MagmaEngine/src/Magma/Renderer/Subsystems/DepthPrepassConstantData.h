#pragma once

#include <glm/glm.hpp>

namespace Magma
{
	struct DepthPrepassConstantData
	{
		glm::mat4 modelViewProjMatrix;
	};
}