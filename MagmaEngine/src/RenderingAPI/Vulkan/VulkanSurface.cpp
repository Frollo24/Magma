#include "mgmpch.h"
#include "VulkanSurface.h"
#include "VulkanInstance.h"

// TODO handle multiple native platforms
#include <GLFW/glfw3.h>

namespace Magma
{
	VulkanSurface::VulkanSurface(const Scope<GraphicsInstance>& instance)
	{
		const VulkanInstance* vkInstanceCast = dynamic_cast<VulkanInstance*>(instance.get());
		MGM_CORE_ASSERT(vkInstanceCast, "Invalid Graphics Instance!");
		m_InstanceHandle = vkInstanceCast->GetInstanceHandle();
		GLFWwindow* windowHandle = static_cast<GLFWwindow*>(vkInstanceCast->GetWindowHandle());
		VkResult result = glfwCreateWindowSurface(m_InstanceHandle, windowHandle, nullptr, &m_SurfaceHandle);
		MGM_CORE_VERIFY(result == VK_SUCCESS);
	}

	VulkanSurface::~VulkanSurface()
	{
		vkDestroySurfaceKHR(m_InstanceHandle, m_SurfaceHandle, nullptr);
	}
}