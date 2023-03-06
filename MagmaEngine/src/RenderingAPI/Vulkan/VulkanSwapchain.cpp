#include "mgmpch.h"
#include "VulkanSwapchain.h"

#include "VulkanDevice.h"
#include "VulkanSurface.h"
#include <GLFW/glfw3.h>

namespace Magma
{
	VulkanSwapchain::VulkanSwapchain(const Ref<RenderDevice>& device, RenderSurface& surface, void* window)
		: m_Device(DynamicCast<VulkanDevice>(device)->GetLogicalDevice()), m_Surface(dynamic_cast<VulkanSurface*>(&surface)->GetHandle())
	{
		const auto& vulkanDevice = DynamicCast<VulkanDevice>(device);
		Create(vulkanDevice, surface, window);
	}

	VulkanSwapchain::VulkanSwapchain(const Ref<RenderDevice>& device, const Scope<RenderSurface>& surface, void* window)
		: m_Device(DynamicCast<VulkanDevice>(device)->GetLogicalDevice()), m_Surface(dynamic_cast<VulkanSurface*>(surface.get())->GetHandle())
	{
		const auto& vulkanDevice = DynamicCast<VulkanDevice>(device);
		Create(vulkanDevice, *surface.get(), window);
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		Destroy();
	}

	SwapchainSupportDetails VulkanSwapchain::QuerySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		SwapchainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.SurfaceCapabilities);

		u32 formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
		if (formatCount) {
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.Formats.data());
		}

		u32 presentCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentCount, nullptr);
		if (presentCount) {
			details.PresentModes.resize(presentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentCount, details.PresentModes.data());
		}

		return details;
	}

	void VulkanSwapchain::Create(const Ref<VulkanDevice>& device, RenderSurface& surface, void* window)
	{
		const auto& swapchainSupport = QuerySwapchainSupportDetails(device->GetPhysicalDevice(), m_Surface);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainFormat(swapchainSupport.Formats);
		VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(swapchainSupport.PresentModes);
		VkExtent2D imageExtent = ChooseSwapchainExtent(swapchainSupport.SurfaceCapabilities, window);

		m_SurfaceFormat = surfaceFormat;
		m_Extent = imageExtent;

		u32 imageCount = swapchainSupport.SurfaceCapabilities.minImageCount + 1;
		if (swapchainSupport.SurfaceCapabilities.maxImageCount > 0 && imageCount > swapchainSupport.SurfaceCapabilities.maxImageCount)
			imageCount = swapchainSupport.SurfaceCapabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = imageExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		VulkanDeviceQueueIndices indices = device->GetQueueIndices();
		std::array<u32, 2> queueFamilyIndices = { indices.GraphicsQueue, indices.PresentQueue };

		if (indices.GraphicsQueue != indices.PresentQueue) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = static_cast<u32>(queueFamilyIndices.size());
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapchainSupport.SurfaceCapabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkResult result = vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain);
		MGM_CORE_VERIFY(result == VK_SUCCESS);
		MGM_CORE_INFO("Successfully created Render Swapchain!");
	}

	void VulkanSwapchain::Destroy()
	{
		vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
	}

	VkSurfaceFormatKHR VulkanSwapchain::ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
	{
		for (const auto& surfaceFormat : availableFormats)
			if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return surfaceFormat;

		return availableFormats[0];
	}
	VkPresentModeKHR VulkanSwapchain::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const
	{
		for (const auto& presentMode : availablePresentModes)
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return presentMode;

		return VK_PRESENT_MODE_FIFO_KHR;
	}
	VkExtent2D VulkanSwapchain::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, void* window) const
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		int width;
		int height;
		glfwGetFramebufferSize((GLFWwindow*)window, &width, &height);

		VkExtent2D actualExtent =
		{
			static_cast<u32>(width),
			static_cast<u32>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}