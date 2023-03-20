#include "mgmpch.h"
#include "GlfwWindow.h"

#include "Magma/Window/WindowSystem.h"
#include "Magma/Events/ApplicationEvent.h"
#include "Magma/Events/WindowEvent.h"
#include "Magma/Events/KeyEvent.h"
#include "Magma/Events/MouseEvent.h"

#include "Magma/Renderer/RenderSwapchain.h"

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
		if (m_Data.WasResized)
		{
			while (m_Data.Minimized)
			{
				glfwWaitEvents();
			}

			m_Data.WasResized = false;
			m_Instance->GetSwapchain()->Invalidate(m_Window);
		}
		else
		{
			m_Instance->PresentFrame();
		}
	}

	void GlfwWindow::SetVSync(bool enabled)
	{
		m_Data.VSync = enabled;
		m_Instance->SetVSync(enabled);
	}

	void GlfwWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		MGM_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);
		m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_Window, &m_Data);

		m_Instance = GraphicsInstance::Create(m_Window);
		m_Surface = RenderSurface::Create(m_Instance);
		m_Instance->Init(m_Surface);
		SetVSync(true);

		//Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.Width = width;
		data.Height = height;
		data.WasResized = true;

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

		static int repeatCount = 1;
		switch (action)
		{
			case GLFW_PRESS:
			{
				KeyPressedEvent event((KeyCode)key, 0);
				data.EventCallback(event);
				repeatCount = 1;
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event((KeyCode)key);
				data.EventCallback(event);
				repeatCount = 1;
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event((KeyCode)key, repeatCount);
				data.EventCallback(event);
				repeatCount++;
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
		m_Instance->Shutdown();
		m_Surface = nullptr;
		m_Instance = nullptr;
		glfwDestroyWindow(m_Window);
	}
}