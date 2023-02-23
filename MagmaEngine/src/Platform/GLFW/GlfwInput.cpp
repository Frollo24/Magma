#include "mgmpch.h"
#include "GlfwInput.h"

#include "Magma/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Magma
{
	Input* Input::s_Instance = new GlfwInput();

	bool GlfwInput::IsKeyDownImpl(KeyCode keycode)
	{
		i32 key = static_cast<i32>(keycode);
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Instance().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, key);
		if (s_KeyMaps.find(key) == s_KeyMaps.end()) s_KeyMaps[key] = GLFW_PRESS;
		bool keyState = s_KeyMaps[key];
		return state == GLFW_PRESS && !keyState;
	}

	bool GlfwInput::IsKeyPressedImpl(KeyCode keycode)
	{
		i32 key = static_cast<i32>(keycode);
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Instance().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, key);
		return state == GLFW_PRESS;
	}

	bool GlfwInput::IsKeyReleasedImpl(KeyCode keycode)
	{
		i32 key = static_cast<i32>(keycode);
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Instance().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, key);
		return state == GLFW_RELEASE;
	}

	bool GlfwInput::IsKeyUpImpl(KeyCode keycode)
	{
		i32 key = static_cast<i32>(keycode);
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Instance().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, key);
		if (s_KeyMaps.find(key) == s_KeyMaps.end()) s_KeyMaps[key] = GLFW_RELEASE;
		bool keyState = s_KeyMaps[key];
		return state == GLFW_RELEASE && keyState;
	}

	bool GlfwInput::IsMouseButtonPressedImpl(MouseCode button)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Instance().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, static_cast<i32>(button));
		return state == GLFW_PRESS;
	}

	bool GlfwInput::IsMouseButtonReleasedImpl(MouseCode button)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Instance().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, static_cast<i32>(button));
		return state == GLFW_RELEASE;
	}

	glm::vec2 GlfwInput::GetMousePositionImpl()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Instance().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float GlfwInput::GetMouseXImpl()
	{
		return GetMousePositionImpl().x;
	}

	float GlfwInput::GetMouseYImpl()
	{
		return GetMousePositionImpl().y;
	}

	void GlfwInput::OnUpdateImpl()
	{
		for (auto& [key, state] : s_KeyMaps)
		{
			GLFWwindow* window = static_cast<GLFWwindow*>(Application::Instance().GetWindow().GetNativeWindow());
			auto newstate = glfwGetKey(window, key);
			s_KeyMaps[key] = newstate;
		}
	}
}