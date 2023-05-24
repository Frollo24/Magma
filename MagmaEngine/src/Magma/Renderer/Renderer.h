#pragma once

#include "Magma/Core/Base.h"

// TEMPORARY
#include "Magma/Renderer/Texture.h"

namespace Magma
{
	class MAGMA_API Renderer
	{
	public:
		static void Init();
		static void BeginFrame();
		static void DrawToScreen();
		static void EndFrame();
		static void Shutdown();

		// TEMPORARY
		static void SetScreenTexture(const Ref<FramebufferTexture2D>& screenTexture);

	private:
		static bool s_BegunFrame;
	};
}

