#pragma once

#include <glm/glm.hpp>

namespace Magma
{
	struct SSAOConstantData
	{
		glm::vec4 samples[64];
	};
}