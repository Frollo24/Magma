#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderDevice.h"
#include "Magma/Renderer/Framebuffer.h"

#include <glm/glm.hpp>

namespace Magma
{
	enum class AttachmentFormat
	{
		None,
		R8,
		RGBA8,
		RGBA16F,
		D24S8,
		D32
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

		inline virtual u32 GetColorAttachmentsCount() = 0;

		inline const RenderPassSpecification& GetSpecification() const { return m_Specification; }

		inline const Ref<Framebuffer>& GetFramebuffer() const { return m_Framebuffer; }
		inline void SetFramebuffer(const Ref<Framebuffer>& framebuffer) { m_Framebuffer = framebuffer; }

	protected:
		explicit RenderPass(const RenderPassSpecification& spec) : m_Specification(spec) {}
		RenderPassSpecification m_Specification;
		Ref<Framebuffer> m_Framebuffer = nullptr;
	};
}

