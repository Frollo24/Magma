#include "mgmpch.h"
#include "Application.h"

#include "Magma/Window/WindowSystem.h"
#include "Magma/Renderer/Renderer.h"

namespace Magma
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
		WindowSystem::Init();
		m_Window = WindowSystem::Create();
		m_Window->SetEventCallback(MGM_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();
	}

	Application::~Application()
	{
		Renderer::Shutdown();

		m_LayerStack.Clear();

		WindowSystem::Destroy(m_Window);
		WindowSystem::Shutdown();
		s_Instance = nullptr;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			float time = m_Window->GetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			Time::SetTotalTime(time);
			Time::SetDeltaTime(timestep);

			Renderer::BeginFrame();
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();
			Renderer::EndFrame();

			Input::OnUpdate();
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(MGM_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(MGM_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled()) break;
			(*it)->OnEvent(e);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		if (layer) m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		if (overlay) m_LayerStack.PushOverlay(overlay);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		MGM_CORE_WARN("{0}", e);
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Window->SetMinimized(true);
			return false;
		}

		m_Window->SetMinimized(false);
		return false;
	}
}