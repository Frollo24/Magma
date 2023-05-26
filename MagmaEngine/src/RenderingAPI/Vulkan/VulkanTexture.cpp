#include "mgmpch.h"
#include "VulkanTexture.h"

#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"

#include <stb_image.h>

namespace Magma
{
	VulkanTexture2D::VulkanTexture2D(const Ref<RenderDevice>& device, const std::string& filepath, const bool generateMipmapsOnLoad)
		: m_VulkanDevice(DynamicCast<VulkanDevice>(device))
	{
		m_PhysicalDevice = m_VulkanDevice->GetPhysicalDevice();
		m_Device = m_VulkanDevice->GetLogicalDevice();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		MGM_CORE_ASSERT(pixels, "Failed to load image!");
		m_MipLevels = generateMipmapsOnLoad ? static_cast<u32>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;

		VkDeviceSize imageSize = (u64)width * height * 4;
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
		CreateBuffer(m_PhysicalDevice, m_Device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, imageSize);
		vkUnmapMemory(m_Device, stagingBufferMemory);

		stbi_image_free(pixels);

		m_ImageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		m_Dimensions = { (u32)width, (u32)height, 1 };

		VulkanImageProperties imageProperties{};
		imageProperties.extent = { (u32)width, (u32)height, 1 };
		imageProperties.type = VK_IMAGE_TYPE_2D;
		imageProperties.numSamples = VK_SAMPLE_COUNT_1_BIT;
		imageProperties.format = m_ImageFormat;
		imageProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageProperties.mipLevels = m_MipLevels;

		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		if (m_MipLevels > 1)
			imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		CreateImage(m_PhysicalDevice, m_Device, imageProperties, imageUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory);

		VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(RenderContext::Instance().get());
		VkCommandPool commandPool = vulkanContext->GetCommandPool();

		VkCommandBuffer imageCreationCommand = BeginSingleTimeCommands(commandPool, m_Device);
		TransitionImageLayout(imageCreationCommand, m_Image, FramebufferTextureFormat::Color,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
		CopyBufferToImage(imageCreationCommand, stagingBuffer, m_Image, (u32)width, (u32)height);
		TransitionImageLayout(imageCreationCommand, m_Image, FramebufferTextureFormat::Color,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_MipLevels);
		EndSingleTimeCommands(commandPool, m_Device, imageCreationCommand, m_VulkanDevice->GetQueueHandles().GraphicsQueue);

		vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = m_MipLevels;
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
		samplerInfo.maxLod = static_cast<f32>(m_MipLevels);

		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;

		result = vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler);
		MGM_CORE_ASSERT(result == VK_SUCCESS, "Failed to create texture sampler!");

		if (generateMipmapsOnLoad)
			GenerateMipmaps();
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

	void VulkanTexture2D::GenerateMipmaps()
	{
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, m_ImageFormat, &formatProperties);

		MGM_CORE_ASSERT(
			(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT),
			"Texture image format does not support linear blitting!"
		);

		VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(RenderContext::Instance().get());
		VkCommandPool commandPool = vulkanContext->GetCommandPool();

		// We change the image layout before generating mipmaps
		VkCommandBuffer transitionCommand = BeginSingleTimeCommands(commandPool, m_Device);
		TransitionImageLayout(transitionCommand, m_Image, FramebufferTextureFormat::Color,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
		EndSingleTimeCommands(commandPool, m_Device, transitionCommand, m_VulkanDevice->GetQueueHandles().GraphicsQueue);

		VkCommandBuffer mipmapCommand = BeginSingleTimeCommands(commandPool, m_Device);
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = m_Image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = m_Dimensions.width;
		int32_t mipHeight = m_Dimensions.height;

		for (uint32_t i = 1; i < m_MipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(mipmapCommand,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(mipmapCommand,
				m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(mipmapCommand,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = m_MipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(mipmapCommand,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommands(commandPool, m_Device, mipmapCommand, m_VulkanDevice->GetQueueHandles().GraphicsQueue);
	}

	static VkFormat FramebufferTextureFormatToVkFormat(const FramebufferTextureFormat& format)
	{
		switch (format)
		{
			case FramebufferTextureFormat::RGBA8:        return VK_FORMAT_R8G8B8A8_SRGB;
			case FramebufferTextureFormat::Color:        return VK_FORMAT_R8G8B8A8_SRGB;
			case FramebufferTextureFormat::RGBA16F:      return VK_FORMAT_R16G16B16A16_SFLOAT;
			case FramebufferTextureFormat::Depth:        return VK_FORMAT_D32_SFLOAT;
			case FramebufferTextureFormat::DepthStencil: return VK_FORMAT_D24_UNORM_S8_UINT;
			default: return VK_FORMAT_UNDEFINED;
		}
	}

	static VkImageUsageFlags FramebufferTextureFormatToVkAttachmentFlag(const FramebufferTextureFormat& format)
	{
		switch (format)
		{
			case FramebufferTextureFormat::RGBA8:        return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			case FramebufferTextureFormat::Color:        return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			case FramebufferTextureFormat::RGBA16F:      return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			case FramebufferTextureFormat::Depth:        return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			case FramebufferTextureFormat::DepthStencil: return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			default: return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
		}
	}

	static VkImageLayout FramebufferTextureFormatToVkImageLayout(const FramebufferTextureFormat& format)
	{
		switch (format)
		{
			case FramebufferTextureFormat::RGBA8:        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case FramebufferTextureFormat::Color:        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case FramebufferTextureFormat::RGBA16F:      return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case FramebufferTextureFormat::Depth:        return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			case FramebufferTextureFormat::DepthStencil: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			default: return VK_IMAGE_LAYOUT_MAX_ENUM;
		}
	}

	VulkanFramebufferTexture2D::VulkanFramebufferTexture2D(const Ref<RenderDevice>& device, FramebufferTextureFormat format, const u32 width, const u32 height, const u32 numSamples)
		: m_VulkanDevice(DynamicCast<VulkanDevice>(device))
	{
		m_PhysicalDevice = m_VulkanDevice->GetPhysicalDevice();
		m_Device = m_VulkanDevice->GetLogicalDevice();
		m_Dimensions = { width, height, 1 };

		VkFormat vkformat = FramebufferTextureFormatToVkFormat(format);
		VkImageUsageFlags attachment = FramebufferTextureFormatToVkAttachmentFlag(format);
		VkImageAspectFlags aspect = FramebufferTextureFormatToVkImageAspect(format);

		VulkanImageProperties imageProperties{};
		imageProperties.extent = { width, height, 1 };
		imageProperties.type = VK_IMAGE_TYPE_2D;
		imageProperties.numSamples = (VkSampleCountFlagBits)numSamples;
		imageProperties.format = vkformat;
		imageProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageProperties.mipLevels = 1;

		VkImageUsageFlags imageUsage = attachment | VK_IMAGE_USAGE_SAMPLED_BIT;
		CreateImage(m_PhysicalDevice, m_Device, imageProperties, imageUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory);

		VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(RenderContext::Instance().get());
		VkCommandPool commandPool = vulkanContext->GetCommandPool();

		VkCommandBuffer imageCreationCommand = BeginSingleTimeCommands(commandPool, m_Device);
		TransitionImageLayout(imageCreationCommand, m_Image, format,
			VK_IMAGE_LAYOUT_UNDEFINED, FramebufferTextureFormatToVkImageLayout(format));
		EndSingleTimeCommands(commandPool, m_Device, imageCreationCommand, m_VulkanDevice->GetQueueHandles().GraphicsQueue);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = vkformat;
		viewInfo.subresourceRange.aspectMask = aspect;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView);
		MGM_CORE_ASSERT(result == VK_SUCCESS, "Failed to create framebuffer texture imageview!");

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		result = vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler);
		MGM_CORE_ASSERT(result == VK_SUCCESS, "Failed to create framebuffer texture sampler!");
	}

	VulkanFramebufferTexture2D::~VulkanFramebufferTexture2D()
	{
		if (m_VulkanDevice == nullptr) return; // Is a thin wrapper to a swapchain image view, don't destroy anything

		vkDestroySampler(m_Device, m_Sampler, nullptr);
		vkDestroyImageView(m_Device, m_ImageView, nullptr);
		vkDestroyImage(m_Device, m_Image, nullptr);
		vkFreeMemory(m_Device, m_ImageMemory, nullptr);
	}

	Ref<VulkanFramebufferTexture2D> VulkanFramebufferTexture2D::CreateFromImageView(const VkImageView& imageView)
	{
		return CreateRef<VulkanFramebufferTexture2D>(imageView);
	}

	VulkanTextureCube::VulkanTextureCube(const Ref<RenderDevice>& device, const std::array<std::string, 6>& filepaths, const bool generateMipmapsOnLoad)
		: m_VulkanDevice(DynamicCast<VulkanDevice>(device))
	{
		m_PhysicalDevice = m_VulkanDevice->GetPhysicalDevice();
		m_Device = m_VulkanDevice->GetLogicalDevice();

		stbi_uc* cubeLayers[6];
		int width, height, channels, layer = 0;
		for (const auto& filepath : filepaths)
		{
			stbi_set_flip_vertically_on_load(0);
			stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
			MGM_CORE_ASSERT(pixels, "Failed to load image from cubemap!");
			cubeLayers[layer] = pixels;
			layer++;
		}
		m_MipLevels = generateMipmapsOnLoad ? static_cast<u32>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;

		VkDeviceSize imageSize = (u64)width * height * 4 * 6;
		VkDeviceSize layerSize = imageSize / 6;
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
		CreateBuffer(m_PhysicalDevice, m_Device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
		sz direction = (sz)data;
		for (int i = 0; i < 6; i++)
		{
			memcpy((void*)(direction + layerSize * i), cubeLayers[i], layerSize);
		}
		vkUnmapMemory(m_Device, stagingBufferMemory);

		for (stbi_uc* image : cubeLayers)
			stbi_image_free(image);

		m_ImageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		m_Dimensions = { (u32)width, (u32)height, 1 };

		VulkanImageProperties imageProperties{};
		imageProperties.extent = { (u32)width, (u32)height, 1 };
		imageProperties.type = VK_IMAGE_TYPE_2D;
		imageProperties.numSamples = VK_SAMPLE_COUNT_1_BIT;
		imageProperties.format = m_ImageFormat;
		imageProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageProperties.mipLevels = m_MipLevels;
		imageProperties.layers = 6;

		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		if (m_MipLevels > 1)
			imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		CreateImage(m_PhysicalDevice, m_Device, imageProperties, imageUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory);

		VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(RenderContext::Instance().get());
		VkCommandPool commandPool = vulkanContext->GetCommandPool();

		VkCommandBuffer imageCreationCommand = BeginSingleTimeCommands(commandPool, m_Device);
		TransitionImageLayout(imageCreationCommand, m_Image, FramebufferTextureFormat::Color,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels, 6);
		CopyBufferToImage(imageCreationCommand, stagingBuffer, m_Image, (u32)width, (u32)height, 6);
		TransitionImageLayout(imageCreationCommand, m_Image, FramebufferTextureFormat::Color,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_MipLevels, 6);
		EndSingleTimeCommands(commandPool, m_Device, imageCreationCommand, m_VulkanDevice->GetQueueHandles().GraphicsQueue);

		vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = m_MipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;

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
		samplerInfo.maxLod = static_cast<f32>(m_MipLevels);

		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;

		result = vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler);
		MGM_CORE_ASSERT(result == VK_SUCCESS, "Failed to create texture sampler!");

		if (generateMipmapsOnLoad)
			GenerateMipmaps();
	}

	VulkanTextureCube::~VulkanTextureCube()
	{
		vkDestroySampler(m_Device, m_Sampler, nullptr);
		vkDestroyImageView(m_Device, m_ImageView, nullptr);
		vkDestroyImage(m_Device, m_Image, nullptr);
		vkFreeMemory(m_Device, m_ImageMemory, nullptr);
	}

	void VulkanTextureCube::SetData(const void* data, u32 size)
	{
	}

	void VulkanTextureCube::GenerateMipmaps()
	{
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, m_ImageFormat, &formatProperties);

		MGM_CORE_ASSERT(
			(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT),
			"Texture image format does not support linear blitting!"
		);

		VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(RenderContext::Instance().get());
		VkCommandPool commandPool = vulkanContext->GetCommandPool();

		// We change the image layout before generating mipmaps
		VkCommandBuffer transitionCommand = BeginSingleTimeCommands(commandPool, m_Device);
		TransitionImageLayout(transitionCommand, m_Image, FramebufferTextureFormat::Color,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels, 6);
		EndSingleTimeCommands(commandPool, m_Device, transitionCommand, m_VulkanDevice->GetQueueHandles().GraphicsQueue);

		VkCommandBuffer mipmapCommand = BeginSingleTimeCommands(commandPool, m_Device);
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = m_Image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 6;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = m_Dimensions.width;
		int32_t mipHeight = m_Dimensions.height;

		for (uint32_t i = 1; i < m_MipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(mipmapCommand,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 6;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 6;

			vkCmdBlitImage(mipmapCommand,
				m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(mipmapCommand,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = m_MipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(mipmapCommand,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommands(commandPool, m_Device, mipmapCommand, m_VulkanDevice->GetQueueHandles().GraphicsQueue);
	}
}