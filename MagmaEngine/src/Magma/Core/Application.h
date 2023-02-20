#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Window/Window.h"

namespace Magma
{
	class MAGMA_API Application
	{
	public:
		Application();
		virtual ~Application();

		inline static Application& Instance() { return *s_Instance; }

		void Run();

	private:
		static Application* s_Instance;
		bool m_Running = true;
		Scope<Window> m_Window;
	};

	// To be defined by client
	Application* CreateApplication();
}
