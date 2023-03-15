#pragma once

#include "Magma/Core/Base.h"

namespace Magma
{
	class MAGMA_API Renderer
	{
	public:
		static void Init();
		static void BeginFrame();
		static void EndFrame();
		static void Shutdown();

	private:
		static bool s_BegunFrame;
	};
}
