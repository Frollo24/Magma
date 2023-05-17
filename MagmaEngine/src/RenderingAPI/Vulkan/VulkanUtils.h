#pragma once

#include "Magma/Core/Base.h"

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
}