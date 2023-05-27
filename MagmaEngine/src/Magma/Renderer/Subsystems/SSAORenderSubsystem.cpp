#include "mgmpch.h"
#include "SSAORenderSubsystem.h"

#include "Magma/Renderer/Renderer.h"

namespace Magma
{
	SSAORenderSubsystem::SSAORenderSubsystem(const Ref<Pipeline>& pipeline)
		: RenderSubsystem(pipeline)
	{
	}

	SSAORenderSubsystem::~SSAORenderSubsystem()
	{
		m_Pipeline = nullptr;
	}

	bool SSAORenderSubsystem::IsGameObjectSuitable(const Ref<GameObject>& gameObject)
	{
		return true;
	}

	void SSAORenderSubsystem::RenderGameObjects()
	{
		RenderCommand::DrawVertices(6, 1, 0, 0);
	}

	void SSAORenderSubsystem::CalculateKernelPositions(SSAOConstantData& ssaoData)
	{
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator;

		for (int i = 0; i < 64; i++)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			ssaoData.samples[i] = glm::vec4(sample, 1.0f);
		}
	}
}