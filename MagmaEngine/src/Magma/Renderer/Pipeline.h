#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderDevice.h"
#include "Magma/Renderer/RenderPass.h"
#include "Magma/Renderer/Shader.h"
#include "Magma/Renderer/Descriptors.h"
#include "Magma/Renderer/PipelineState.h"
#include "Magma/Renderer/InputElementsLayout.h"

namespace Magma
{
	enum class PipelineType
	{
		Graphics
	};

	struct GlobalDataLayout
	{
		std::vector<Ref<DescriptorSetLayout>> DescriptorLayouts{};
		u32 ConstantDataSize = 0;
	};

	struct PipelineSpecification
	{
		PipelineType PipelineType = PipelineType::Graphics;
		PipelineDepthState PipelineDepthState{};
		PipelineBlendState PipelineBlendState{};
		PipelinePolygonState PipelinePolygonState{};
		InputElementsLayout InputElementsLayout{};
		GlobalDataLayout GlobalDataLayout{};

		Ref<Shader> Shader = nullptr;
	};

	class MAGMA_API Pipeline
	{
	public:
		virtual ~Pipeline() = default;

		static Ref<Pipeline> Create(const PipelineSpecification& spec, const Ref<RenderDevice>& device, const Ref<RenderPass>& renderPass);

		inline const PipelineSpecification& GetSpecification() const { return m_Specification; }
		inline virtual Ref<RenderPass> GetRenderPass() const = 0;

	protected:
		explicit Pipeline(const PipelineSpecification& spec) : m_Specification(spec) {}
		PipelineSpecification m_Specification;
	};
}

