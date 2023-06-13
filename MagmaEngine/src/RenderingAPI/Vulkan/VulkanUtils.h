#pragma once

#include "Magma/Core/Base.h"

// TEMPORARY: Texture Format
#include "VulkanFramebuffer.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	static VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool, VkDevice device)
	{
		VkCommandBuffer singleTimeCommandBuffer = VK_NULL_HANDLE;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		
		VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &singleTimeCommandBuffer);
		MGM_CORE_VERIFY(result == VK_SUCCESS);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		result = vkBeginCommandBuffer(singleTimeCommandBuffer, &beginInfo);
		MGM_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin recording single time command buffer!");

		return singleTimeCommandBuffer;
	}

	static void EndSingleTimeCommands(VkCommandPool commandPool, VkDevice device, VkCommandBuffer singleTimeCommandBuffer, VkQueue submissionQueue)
	{
		vkEndCommandBuffer(singleTimeCommandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &singleTimeCommandBuffer;

		vkQueueSubmit(submissionQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(submissionQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &singleTimeCommandBuffer);
	}

	static u32 FindMemoryType(const VkPhysicalDevice& physicalDevice, const VkMemoryRequirements& requirements, const VkMemoryPropertyFlags& flags)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
			if ((requirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags)
				return i;

		MGM_CORE_ASSERT(false, "Failed to find suitable memory");
		return UINT32_MAX;
	}

	static void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
	{
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = size;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(device, &createInfo, nullptr, &buffer);
		MGM_CORE_VERIFY(result == VK_SUCCESS);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements, properties);

		result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
		MGM_CORE_VERIFY(result == VK_SUCCESS);
		vkBindBufferMemory(device, buffer, memory, 0);
	}

	struct VulkanImageProperties
	{
		VkExtent3D extent;
		VkImageType type;
		VkSampleCountFlagBits numSamples;
		VkFormat format;
		VkImageTiling tiling;
		u32 mipLevels = 1;
		u32 layers = 1;
	};

	static void CreateImage(VkPhysicalDevice physicalDevice, VkDevice device, VulkanImageProperties imageProperties,
		VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImage& image, VkDeviceMemory& memory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = imageProperties.type;
		imageInfo.extent = imageProperties.extent;
		imageInfo.mipLevels = imageProperties.mipLevels;
		imageInfo.arrayLayers = imageProperties.layers;
		imageInfo.format = imageProperties.format;
		imageInfo.tiling = imageProperties.tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.samples = imageProperties.numSamples;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (imageProperties.layers == 6)
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VkResult result = vkCreateImage(device, &imageInfo, nullptr, &image);
		MGM_CORE_VERIFY(result == VK_SUCCESS);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements, memoryProperties);

		result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
		MGM_CORE_VERIFY(result == VK_SUCCESS);

		vkBindImageMemory(device, image, memory, 0);
	}

	static VkImageAspectFlags FramebufferTextureFormatToVkImageAspect(const FramebufferTextureFormat& format)
	{
		switch (format)
		{
			case FramebufferTextureFormat::Red:            return VK_IMAGE_ASPECT_COLOR_BIT;
			case FramebufferTextureFormat::RGBA8:          return VK_IMAGE_ASPECT_COLOR_BIT;
			case FramebufferTextureFormat::Color:          return VK_IMAGE_ASPECT_COLOR_BIT;
			case FramebufferTextureFormat::RGBA16F:        return VK_IMAGE_ASPECT_COLOR_BIT;
			case FramebufferTextureFormat::Depth:          return VK_IMAGE_ASPECT_DEPTH_BIT;
			case FramebufferTextureFormat::DepthStencil:   return (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT );
			default: return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
		}
	}

	static VkAccessFlags VkImageLayoutToVkAccessFlags(const VkImageLayout& layout)
	{
		switch (layout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:	                       return VK_ACCESS_NONE_KHR;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:             return VK_ACCESS_TRANSFER_WRITE_BIT;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:         return VK_ACCESS_SHADER_READ_BIT;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:         return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:         return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			default: return VK_ACCESS_FLAG_BITS_MAX_ENUM;
		}
	}

	static VkPipelineStageFlags VkImageLayoutToVkPipelineStageFlags(const VkImageLayout& layout)
	{
		switch (layout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:	                       return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:             return VK_PIPELINE_STAGE_TRANSFER_BIT;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:         return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:         return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:         return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			default: return VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;
		}
	}

	static void TransitionImageLayout(VkCommandBuffer transitionCommand, VkImage image, FramebufferTextureFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, u32 mipLevels = 1, u32 layers = 1)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		VkImageAspectFlags aspectMask = FramebufferTextureFormatToVkImageAspect(format);

		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectMask;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layers;

		VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE_KHR;
		VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE_KHR;

		barrier.srcAccessMask = VkImageLayoutToVkAccessFlags(oldLayout);
		barrier.dstAccessMask = VkImageLayoutToVkAccessFlags(newLayout);
		sourceStage = VkImageLayoutToVkPipelineStageFlags(oldLayout);
		destinationStage = VkImageLayoutToVkPipelineStageFlags(newLayout);

		MGM_CORE_ASSERT(barrier.srcAccessMask != VK_ACCESS_FLAG_BITS_MAX_ENUM, "Unsupported old layout transition!");
		MGM_CORE_ASSERT(barrier.dstAccessMask != VK_ACCESS_FLAG_BITS_MAX_ENUM, "Unsupported new layout transition!");
		MGM_CORE_ASSERT(sourceStage != VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM, "Unsupported old layout transition!");
		MGM_CORE_ASSERT(destinationStage != VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM, "Unsupported new layout transition!");

		vkCmdPipelineBarrier(
			transitionCommand,
			sourceStage, destinationStage,
			0, 0, nullptr, 0, nullptr, 1, &barrier
		);
	}

	static void CopyBufferToImage(VkCommandBuffer copyCommand, VkBuffer buffer, VkImage image, u32 width, u32 height, u32 layers = 1)
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layers;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(copyCommand, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
}