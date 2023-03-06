#pragma once

#include "Magma/Renderer/GraphicsInstance.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanInstance : public GraphicsInstance
	{
	public:
		VulkanInstance(void* window);
		virtual ~VulkanInstance();

		virtual void Init(const Scope<RenderSurface>& surface) override;
		virtual void Shutdown() override;
		virtual void SetVSync(bool enabled) override;
		virtual void PresentFrame() override;

		inline void* GetWindowHandle() const { return m_WindowHandle; }
		inline VkInstance GetInstanceHandle() const { return m_Instance; }

	private:
		static std::vector<const char*> GetRequiredExtensions();
		static void CheckAvailableExtensions();

	private:
		void* m_WindowHandle = nullptr;
		VkInstance m_Instance = VK_NULL_HANDLE;
	};
}

