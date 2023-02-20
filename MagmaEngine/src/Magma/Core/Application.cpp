#include "mgmpch.h"
#include "Application.h"

#include "Magma/Window/WindowSystem.h"

#include <GLFW/glfw3.h> // TEMPORARY

namespace Magma
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
		WindowSystem::Init();
		m_Window = WindowSystem::Create();
	}

	Application::~Application()
	{
		WindowSystem::Destroy(m_Window);
		WindowSystem::Shutdown();
		s_Instance = nullptr;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			glClearColor(1, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			m_Window->OnUpdate();
		}
	}
}