#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Elements/Camera.h"
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

		inline static const Ref<Camera>& GetCamera() { return s_Camera; }
		inline static void SetCamera(const Ref<Camera>& camera) { s_Camera = camera; }

		inline static void AddRenderSubsystem(const Ref<RenderSubsystem>& system) { s_RenderSubsystems.push_back(system); }
		inline static const std::vector<Ref<RenderSubsystem>>& GetRenderSubsystems() { return s_RenderSubsystems; }

		// TEMPORARY
		static void AddGameObject(const Ref<GameObject>& gameObject);
		static const Ref<FramebufferTexture2D>& GetScreenTexture();
		static void SetScreenTexture(const Ref<FramebufferTexture2D>& screenTexture);
		static const Ref<DescriptorPool>& GetDescriptorPool();
		static const Ref<DescriptorSetLayout>& GetMaterialDescriptorSetLayout();
		static const Ref<RenderDevice>& GetDevice();

		static void Update();

	private:
		static bool s_BegunFrame;
		static Ref<Camera> s_Camera;
		static std::vector<Ref<RenderSubsystem>> s_RenderSubsystems;
	};
}

