#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Core/KeyCode.h"
#include "Magma/Core/MouseCode.h"

#include <glm/glm.hpp>

namespace Magma
{
	class MAGMA_API Input
	{
	public:
		inline static bool IsKeyDown(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsKeyPressed(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsKeyReleased(KeyCode keycode) { return s_Instance->IsKeyReleasedImpl(keycode); }
		inline static bool IsKeyUp(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }

		inline static bool IsMouseButtonPressed(MouseCode button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static bool IsMouseButtonReleased(MouseCode button) { return s_Instance->IsMouseButtonReleasedImpl(button); }
		inline static glm::vec2 GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

	protected:
		virtual bool IsKeyDownImpl(KeyCode keycode) = 0;
		virtual bool IsKeyPressedImpl(KeyCode keycode) = 0;
		virtual bool IsKeyReleasedImpl(KeyCode keycode) = 0;
		virtual bool IsKeyUpImpl(KeyCode keycode) = 0;

		virtual bool IsMouseButtonPressedImpl(MouseCode button) = 0;
		virtual bool IsMouseButtonReleasedImpl(MouseCode button) = 0;
		virtual glm::vec2 GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

		virtual void OnUpdateImpl() = 0;

		std::unordered_map<int, bool> s_KeyMaps;
		std::unordered_map<int, bool> s_MouseMaps;
	private:
		friend class Application;

		inline static void OnUpdate() { return s_Instance->OnUpdateImpl(); }
		static Input* s_Instance;
	};
}

