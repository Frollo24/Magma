#include "mgmpch.h"
#include "Window.h"

#include "WindowSystem.h"
#include "Platform/GLFW/GlfwWindow.h"

namespace Magma
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
		switch (WindowSystem::GetAPI())
		{
			case WindowAPI::None: MGM_CORE_ASSERT(false, "Using no window API is currently supported!"); return nullptr;
			case WindowAPI::GLFW: return CreateScope<GlfwWindow>(props);
		}

		MGM_CORE_ASSERT(false, "Unknown window API!"); return nullptr;
	}
}