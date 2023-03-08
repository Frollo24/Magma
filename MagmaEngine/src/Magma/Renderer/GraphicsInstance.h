#pragma once

#include "Magma/Core/Base.h"

namespace Magma
{
	class RenderSurface;
	class RenderDevice;
	class RenderSwapchain;

	class MAGMA_API GraphicsInstance
	{
	public:
		virtual ~GraphicsInstance() = default;

		virtual void Init(const Scope<RenderSurface>& surface) = 0;
		virtual void Shutdown() = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void PresentFrame() = 0;

		inline const Ref<RenderDevice>& GetDevice() const { return m_Device; }
		inline const Ref<RenderSwapchain>& GetSwapchain() const { return m_Swapchain; }

		static Scope<GraphicsInstance> Create(void* windowHandle);

	protected:
		Ref<RenderDevice> m_Device;
		Ref<RenderSwapchain> m_Swapchain;
	};
}

