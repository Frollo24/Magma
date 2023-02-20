#include "mgmpch.h"
#include "GlfwWindow.h"

#include "Magma/Window/WindowSystem.h"

namespace Magma
{
	void WindowSystem::Init()
	{
		MGM_CORE_ASSERT(!s_Initialized, "Window System already initialized!");
		int success = glfwInit();
		MGM_CORE_ASSERT(success, "Could not initialize GLFW!");
		s_Initialized = true;
	}

	void WindowSystem::Shutdown()
	{
		MGM_CORE_ASSERT(s_Initialized, "Window System was not initialized!");
		glfwTerminate();

		s_Initialized = false;
	}

	GlfwWindow::GlfwWindow(const WindowProps& props)
	{
		Init(props);
	}

	GlfwWindow::~GlfwWindow()
	{
		Shutdown();
	}

	void GlfwWindow::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_Window); // TEMPORARY
	}

	void GlfwWindow::SetVSync(bool enabled)
	{
		glfwSwapInterval(enabled); // TEMPORARY
		m_Data.VSync = enabled;
	}

	void GlfwWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		MGM_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);
		m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window); // TEMPORARY
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);
	}

	void GlfwWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}
}