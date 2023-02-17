#include "Application.h"

#include "Magma/Events/WindowEvent.h"
#include "Magma/Core/Log.h"

namespace Magma
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
	}

	Application::~Application()
	{
		s_Instance = nullptr;
	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		MGM_CORE_TRACE("{0}", e);

		while (true);
	}
}