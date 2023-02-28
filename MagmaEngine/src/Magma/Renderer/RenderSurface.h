#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/GraphicsInstance.h"

namespace Magma
{
	class MAGMA_API RenderSurface
	{
	public:
		virtual ~RenderSurface() = default;

		static Scope<RenderSurface> Create(const Scope<GraphicsInstance>& instance);
	};
}

