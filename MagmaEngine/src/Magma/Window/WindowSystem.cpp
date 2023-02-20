#include "mgmpch.h"
#include "WindowSystem.h"

#include "Platform/GLFW/GlfwWindow.h"

namespace Magma
{
	bool WindowSystem::s_Initialized = false;

	Scope<Window> WindowSystem::Create(const WindowProps& props)
	{
		MGM_ASSERT(s_Initialized, "Cannot create a window without initializing the Window System!");
		return CreateScope<GlfwWindow>(props);
	}

	void WindowSystem::Destroy(Scope<Window>& window)
	{
		window = nullptr;
	}
}