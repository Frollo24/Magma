#pragma once
#include "Magma/Renderer/RenderSurface.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanSurface : public RenderSurface
	{
	public:
		VulkanSurface(const Scope<GraphicsInstance>& instance);
		virtual ~VulkanSurface();

		inline VkSurfaceKHR GetHandle() const { return m_SurfaceHandle; }

	private:
		VkInstance m_InstanceHandle = VK_NULL_HANDLE;
		VkSurfaceKHR m_SurfaceHandle = VK_NULL_HANDLE;
	};
}

