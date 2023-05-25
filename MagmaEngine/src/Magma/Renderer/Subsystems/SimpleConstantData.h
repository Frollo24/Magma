#pragma once

#include <glm/glm.hpp>

namespace Magma
{
	struct SimpleConstantData
	{
		glm::mat4 modelMatrix;
		glm::vec4 tintColor;
	};

	struct TestConstantData
	{
		glm::mat4 viewProj;
		glm::vec4 tintColor;
		glm::vec3 posOffset;
	};
}