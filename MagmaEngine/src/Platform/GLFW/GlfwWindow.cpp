#include "mgmpch.h"
#include "GlfwWindow.h"

#include "Magma/Window/WindowSystem.h"
#include "Magma/Events/ApplicationEvent.h"
#include "Magma/Events/WindowEvent.h"
#include "Magma/Events/KeyEvent.h"
#include "Magma/Events/MouseEvent.h"

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

		//Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowCloseEvent event(data.Title);
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event((KeyCode)key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event((KeyCode)key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event((KeyCode)key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event((MouseCode)button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event((MouseCode)button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	void GlfwWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}
}