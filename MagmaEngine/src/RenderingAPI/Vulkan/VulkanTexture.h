#pragma once
#include "Magma/Renderer/Texture.h"

#include "RenderingAPI/Vulkan/VulkanDevice.h"
#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const Ref<RenderDevice>& device, const std::string& filepath, const bool generateMipmapsOnLoad = true);
		virtual ~VulkanTexture2D();

		virtual void SetData(const void* data, u32 size) override;
		virtual void GenerateMipmaps() override;

		inline const VkImage& GetRawImage() { return m_Image; }
		inline const VkImageView& GetImageView() { return m_ImageView; }
		inline const VkSampler& GetSampler() { return m_Sampler; }

	private:
		Ref<VulkanDevice> m_VulkanDevice = nullptr;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkImage m_Image = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
		VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
	};
}
