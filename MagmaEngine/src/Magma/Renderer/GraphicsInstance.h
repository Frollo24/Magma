#pragma once

#include "Magma/Core/Base.h"

namespace Magma
{
	class MAGMA_API GraphicsInstance
	{
	public:
		virtual ~GraphicsInstance() = default;

		virtual void Init() = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void PresentFrame() = 0;

		static Scope<GraphicsInstance> Create(void* windowHandle);
	};
}

