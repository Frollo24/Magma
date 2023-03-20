#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Events/Event.h"
#include "Magma/Renderer/GraphicsInstance.h"
#include "Magma/Renderer/RenderSurface.h"

namespace Magma
{
	struct WindowProps
	{
		String Title;
		u32 Width;
		u32 Height;

		WindowProps(const String& title = "Magma Engine", u32 width = 1600, u32 height = 900)
			: Title(title), Width(width), Height(height) {}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;
		virtual u32 GetWidth() const = 0;
		virtual u32 GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetMinimized(bool minimized) = 0;
		virtual bool IsMinimized() const = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		inline virtual void* GetNativeWindow() const = 0;
		inline virtual const Scope<GraphicsInstance>& GetGraphicsInstance() const = 0;
		inline virtual const Scope<RenderSurface>& GetRenderSurface() const = 0;
		inline virtual float GetTime() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}

