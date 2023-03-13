#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderDevice.h"

#include <glm/glm.hpp>

namespace Magma
{
	enum class AttachmentFormat
	{
		None,
		RGBA8,
	};

	enum class ClearFlags
	{
		None = 0,
		Color = MGM_BIT(0),
		DepthStencil = MGM_BIT(1),
	};

	MGM_DEFINE_ENUM_FLAG_OPERATORS(ClearFlags);

	struct ClearValues
	{
		glm::vec4 Color = glm::vec4(1.0f);
		float Depth = 1.0f;
		u32 Stencil = 0;
		ClearFlags ClearFlags = ClearFlags::None;
	};

	struct RenderPassSpecification
	{
		std::vector<AttachmentFormat> Attachments{};
		ClearValues ClearValues{};
		u32 Samples = 1;
		bool IsSwapchainTarget = true;
	};

	class MAGMA_API RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		static Ref<RenderPass> Create(const RenderPassSpecification& spec, const Ref<RenderDevice>& device);

		inline const RenderPassSpecification& GetSpecification() const { return m_Specification; }

	protected:
		explicit RenderPass(const RenderPassSpecification& spec) : m_Specification(spec) {}
		RenderPassSpecification m_Specification;
	};
}

