#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/GraphicsInstance.h"
#include "Magma/Renderer/RenderPass.h"
#include "Magma/Window/Window.h"

namespace Magma
{
	class MAGMA_API RenderSwapchain
	{
	public:
		virtual ~RenderSwapchain() = default;

		virtual void CreateFramebuffers(const Ref<RenderDevice>& device, const Ref<RenderPass>& renderPass) = 0;
		virtual void Invalidate(void* window) = 0;
		virtual void PresentFrame() = 0;

		inline virtual u32 GetImageCount() const = 0;
		inline virtual Ref<RenderPass> GetMainRenderPass() const = 0;
		inline virtual u32 GetWidth() const = 0;
		inline virtual u32 GetHeight() const = 0;

		static Ref<RenderSwapchain> Create(const Ref<RenderDevice>& device, RenderSurface& surface, void* window);
		static Ref<RenderSwapchain> Create(const Ref<RenderDevice>& device, const Scope<RenderSurface>& surface, void* window);
	};
}

