#pragma once

#include <glm/glm.hpp>

namespace Magma
{
	struct GlobalUBO
	{
		glm::mat4 viewProj;
		glm::mat4 proj;
		glm::mat4 view;
	};

	struct FogUBO
	{
		glm::vec4 fogColor; // w stands for fog density
	};

	struct PhysicalCameraUBO
	{
		float focalLength;
		float focusingDistance;
		float exposure;
		float aperture;
	};

	struct DirLight {
		glm::vec4 color = glm::vec4(0.0f);
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
		float intensity = 0.0f;
	};

	struct PointLight {
		glm::vec4 color = glm::vec4(0.0f);
		glm::vec3 position = glm::vec3(0.0f);
		float intensity = 0.0f;
	};

	struct SpotLight {
		glm::vec4 color = glm::vec4(0.0f);
		glm::vec3 position = glm::vec3(0.0f);
		float intensity = 0.0f;
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
		float angle = glm::radians(30.0f);
	};

	struct LightsUBO
	{
		DirLight dirLight[4];
		PointLight pointLight[16];
		SpotLight spotLight[8];
	};

	struct DefaultConstantData
	{
		glm::mat4 modelMatrix;
		glm::vec4 tintColor;
		glm::float32 metallic;
		glm::float32 roughness;
		glm::float32 isPBR;
	};
}