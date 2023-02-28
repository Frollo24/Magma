#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Window/Window.h"

namespace Magma
{
	// TODO add support for Win32
	enum class WindowAPI
	{
		None,
		GLFW
	};

	class MAGMA_API WindowSystem
	{
	public:
		static void Init();
		static void Shutdown();

		static Scope<Window> Create(const WindowProps& props = WindowProps());
		static void Destroy(Scope<Window>& window);

		inline static WindowAPI GetAPI() { return s_API; }

	private:
		static WindowAPI s_API;
		static bool s_Initialized;
	};
}

