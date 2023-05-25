#pragma once

#include <glm/glm.hpp>

namespace Magma
{
	struct SimpleCameraUBO
	{
		glm::mat4 viewProj;
		glm::mat4 proj;
		glm::mat4 view;
	};

	struct SimpleDirLightUBO
	{
		glm::vec4 color;
		glm::vec3 direction;
		float intensity;
	};

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