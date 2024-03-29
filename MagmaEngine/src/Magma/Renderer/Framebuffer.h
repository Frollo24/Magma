#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderDevice.h"
#include "Magma/Renderer/Texture.h"

namespace Magma
{
	enum class FramebufferTextureFormat
	{
		None,
		Red,
		RGBA8,
		Color,
		RGBA16F,
		Depth,
		DepthStencil
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureFormat TextureFormat;
	};

	struct FramebufferSpecification
	{
		u32 Width = 0;
		u32 Height = 0;
		std::vector<FramebufferTextureSpecification> TextureSpecs{};
		std::vector<Ref<FramebufferTexture2D>> RenderTargets{};
	};

	class RenderPass;

	class MAGMA_API Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec, const Ref<RenderDevice>& device, const Ref<RenderPass>& renderPass);

		inline const FramebufferSpecification& GetSpecification() const { return m_Specification; }
	
	protected:
		explicit Framebuffer(const FramebufferSpecification& spec) : m_Specification(spec) {}
		FramebufferSpecification m_Specification;
	};
}

