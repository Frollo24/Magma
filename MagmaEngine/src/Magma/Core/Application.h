#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Core/LayerStack.h"
#include "Magma/Core/Time.h"
#include "Magma/Core/Input.h"
#include "Magma/Events/Event.h"
#include "Magma/Events/WindowEvent.h"
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
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		Window& GetWindow() { return *m_Window; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		static Application* s_Instance;
		bool m_Running = true;
		float m_LastFrameTime = 0.0f;

		Scope<Window> m_Window;
		LayerStack m_LayerStack;
	};

	// To be defined by client
	Application* CreateApplication();
}
