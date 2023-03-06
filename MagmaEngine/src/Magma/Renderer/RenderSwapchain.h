#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/GraphicsInstance.h"
#include "Magma/Window/Window.h"

namespace Magma
{
	class MAGMA_API RenderSwapchain
	{
	public:
		virtual ~RenderSwapchain() = default;

		static Ref<RenderSwapchain> Create(const Ref<RenderDevice>& device, RenderSurface& surface, void* window);
		static Ref<RenderSwapchain> Create(const Ref<RenderDevice>& device, const Scope<RenderSurface>& surface, void* window);
	};
}

