#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Window/Window.h"

namespace Magma
{
	class MAGMA_API WindowSystem
	{
	public:
		static void Init();
		static void Shutdown();

		static Scope<Window> Create(const WindowProps& props = WindowProps());
		static void Destroy(Scope<Window>& window);

	private:
		static bool s_Initialized;
	};
}

