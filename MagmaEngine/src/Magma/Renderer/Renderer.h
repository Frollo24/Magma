#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderSubsystem.h"

// TEMPORARY
#include "Magma/Renderer/Texture.h"

namespace Magma
{
	class MAGMA_API Renderer
	{
	public:
		static void Init();
		static void BeginFrame();
		static void RenderGameObjects();
		static void DrawToScreen();
		static void EndFrame();
		static void Shutdown();

		inline static void AddRenderSubsystem(const Ref<RenderSubsystem>& system) { s_RenderSubsystems.push_back(system); }
		inline static const std::vector<Ref<RenderSubsystem>>& GetRenderSubsystems() { return s_RenderSubsystems; }

		// TEMPORARY
		static void AddGameObject(const Ref<GameObject>& gameObject);
		static void SetScreenTexture(const Ref<FramebufferTexture2D>& screenTexture);

	private:
		static bool s_BegunFrame;
		static std::vector<Ref<RenderSubsystem>> s_RenderSubsystems;
	};
}

