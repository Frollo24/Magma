#pragma once
#include "Magma/Renderer/RenderSwapchain.h"

#include "VulkanDevice.h"
#include <vulkan/vulkan.h>

namespace Magma
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class VulkanSwapchain : public RenderSwapchain
	{
	public:
		VulkanSwapchain(const Ref<RenderDevice>& device, RenderSurface& surface, void* window);
		VulkanSwapchain(const Ref<RenderDevice>& device, const Scope<RenderSurface>& surface, void* window);

		virtual ~VulkanSwapchain();

		static SwapchainSupportDetails QuerySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	private:
		void Create(const Ref<VulkanDevice>& device, RenderSurface& surface, void* window);
		void RetrieveSwapchainImages();
		void Destroy();

		VkSurfaceFormatKHR ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
		VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
		VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, void* window) const;

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

		VkSurfaceFormatKHR m_SurfaceFormat{};
		VkExtent2D m_Extent{};

		std::vector<VkImage> m_Images{};
		std::vector<VkImageView> m_ImageViews{};
	};
}

