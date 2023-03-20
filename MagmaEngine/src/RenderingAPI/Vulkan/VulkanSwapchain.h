#pragma once
#include "Magma/Renderer/RenderSwapchain.h"

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
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

		virtual void CreateFramebuffers(const Ref<RenderDevice>& device, const Ref<RenderPass>& renderPass) override;
		virtual void Invalidate(void* window) override;
		virtual void PresentFrame() override;

		inline VkSwapchainKHR GetHandle() const { return m_Swapchain; }
		inline const Ref<VulkanFramebuffer>& GetFramebuffer(u32 index) const { return m_Framebuffers[index]; }
		inline const VkExtent2D& GetExtent() const { return m_Extent; }
		inline virtual const Ref<RenderPass>& GetMainRenderPass() const override { return m_RenderPass; }
		inline virtual u32 GetImageCount() const override { return m_ImageCount; }

		inline const u32& GetImageIndex() const { return m_ImageIndex; }
		inline void SetImageIndex(const u32& imageIndex) { m_ImageIndex = imageIndex; }
		static SwapchainSupportDetails QuerySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	private:
		void Create(const Ref<VulkanDevice>& device, void* window);
		void Destroy();
		void RetrieveSwapchainImages();

		VkSurfaceFormatKHR ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
		VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
		VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, void* window) const;

	private:
		VkDevice m_DeviceHandle = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkSwapchainKHR m_OldSwapchain = VK_NULL_HANDLE;

		VkSurfaceCapabilitiesKHR m_SurfaceCapabilities{};
		VkSurfaceFormatKHR m_SurfaceFormat{};
		VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
		VkSharingMode m_SharingMode = VK_SHARING_MODE_MAX_ENUM;
		VkExtent2D m_Extent{};

		u32 m_ImageIndex = 0;
		u32 m_ImageCount = 0;
		std::vector<VkImage> m_Images{};
		std::vector<VkImageView> m_ImageViews{};

		Ref<VulkanDevice> m_RenderDevice = nullptr;
		Ref<VulkanRenderPass> m_RenderPass = nullptr;
		std::vector<Ref<VulkanFramebuffer>> m_Framebuffers{};
	};
}

