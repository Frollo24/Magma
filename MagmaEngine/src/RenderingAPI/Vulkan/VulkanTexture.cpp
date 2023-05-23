#include "mgmpch.h"
#include "VulkanTexture.h"

#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"

#include <stb_image.h>

namespace Magma
{
	VulkanTexture2D::VulkanTexture2D(const Ref<RenderDevice>& device, const std::string& filepath)
	{
		Ref<VulkanDevice> vulkanDevice = DynamicCast<VulkanDevice>(device);
		VkPhysicalDevice physicalDevice = vulkanDevice->GetPhysicalDevice();
		m_Device = vulkanDevice->GetLogicalDevice();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		MGM_CORE_ASSERT(pixels, "Failed to load image!");

		VkDeviceSize imageSize = (u64)width * height * 4;

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
		CreateBuffer(physicalDevice, m_Device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, imageSize);
		vkUnmapMemory(m_Device, stagingBufferMemory);

		stbi_image_free(pixels);

		VulkanImageProperties imageProperties{};
		imageProperties.extent = { (u32)width, (u32)height, 1 };
		imageProperties.type = VK_IMAGE_TYPE_2D;
		imageProperties.numSamples = VK_SAMPLE_COUNT_1_BIT;
		imageProperties.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
		CreateImage(physicalDevice, m_Device, imageProperties, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory);

		VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(RenderContext::Instance().get());
		VkCommandPool commandPool = vulkanContext->GetCommandPool();

		VkCommandBuffer imageCreationCommand = BeginSingleTimeCommands(commandPool, m_Device);
		TransitionImageLayout(imageCreationCommand, m_Image, FramebufferTextureFormat::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(imageCreationCommand, stagingBuffer, m_Image, (u32)width, (u32)height);
		TransitionImageLayout(imageCreationCommand, m_Image, FramebufferTextureFormat::Color, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		EndSingleTimeCommands(commandPool, m_Device, imageCreationCommand, vulkanDevice->GetQueueHandles().GraphicsQueue);

		vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView);
		MGM_CORE_ASSERT(result == VK_SUCCESS, "Failed to create texture imageview!");

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;

		result = vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler);
		MGM_CORE_ASSERT(result == VK_SUCCESS, "Failed to create texture sampler!");
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		vkDestroySampler(m_Device, m_Sampler, nullptr);
		vkDestroyImageView(m_Device, m_ImageView, nullptr);
		vkDestroyImage(m_Device, m_Image, nullptr);
		vkFreeMemory(m_Device, m_ImageMemory, nullptr);
	}

	void VulkanTexture2D::SetData(const void* data, u32 size)
	{

	}
}