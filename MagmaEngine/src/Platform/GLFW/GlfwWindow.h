#pragma once

#include "Magma/Window/Window.h"

#include <GLFW/glfw3.h>

namespace Magma
{
	class GlfwWindow : public Window
	{
	public:
		GlfwWindow(const WindowProps& props);
		virtual ~GlfwWindow();

		void OnUpdate() override;

		unsigned int GetWidth() const override { return m_Data.Width; }
		unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		inline void SetMinimized(bool minimized) override { m_Data.Minimized = minimized; }
		inline bool IsMinimized() const override { return m_Data.Minimized; }
		void SetVSync(bool enabled) override;
		inline bool IsVSync() const override { return m_Data.VSync; }

		inline virtual void* GetNativeWindow() const override { return m_Window; }
		inline virtual const Scope<GraphicsInstance>& GetGraphicsInstance() const override { return m_Instance; }
		inline virtual const Scope<RenderSurface>& GetRenderSurface() const override { return m_Surface; }
		inline virtual float GetTime() const override { return (float)glfwGetTime(); }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window = nullptr;
		Scope<GraphicsInstance> m_Instance = nullptr;
		Scope<RenderSurface> m_Surface = nullptr;

		struct WindowData
		{
			std::string Title;
			unsigned int Width = 0, Height = 0;
			bool Minimized = false;
			bool WasResized = false;
			bool VSync = false;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}
