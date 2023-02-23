#pragma once

#include "Magma/Core/Input.h"

namespace Magma
{
	class GlfwInput : public Input
	{
	protected:
		virtual bool IsKeyDownImpl(KeyCode keycode) override;
		virtual bool IsKeyPressedImpl(KeyCode keycode) override;
		virtual bool IsKeyReleasedImpl(KeyCode keycode) override;
		virtual bool IsKeyUpImpl(KeyCode keycode) override;

		virtual bool IsMouseButtonPressedImpl(MouseCode button) override;
		virtual bool IsMouseButtonReleasedImpl(MouseCode button) override;
		virtual glm::vec2 GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;

		virtual void OnUpdateImpl() override;
	};
}

