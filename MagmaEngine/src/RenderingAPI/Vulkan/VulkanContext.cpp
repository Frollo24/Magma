#include "mgmpch.h"
#include "VulkanContext.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include "Magma/Core/Application.h"

namespace Magma
{
	void VulkanContext::Init()
	{
		const Scope<GraphicsInstance>& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		Ref<VulkanDevice> device = DynamicCast<VulkanDevice>(instance->GetDevice());
		Ref<VulkanSwapchain> swapchain = DynamicCast<VulkanSwapchain>(instance->GetSwapchain());

		VulkanDeviceQueueIndices queueIndices = device->GetQueueIndices();
		VkDevice logicalDevice = device->GetLogicalDevice();

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueIndices.GraphicsQueue;
		{
			VkResult result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &m_CommandPool);
			MGM_CORE_VERIFY(result == VK_SUCCESS);
		}
		MGM_CORE_INFO("Successfully created Vulkan Command Pool!");

		m_CommandBuffers.resize(swapchain->GetImageCount());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<u32>(m_CommandBuffers.size());
		{
			VkResult result = vkAllocateCommandBuffers(logicalDevice, &allocInfo, m_CommandBuffers.data());
			MGM_CORE_VERIFY(result == VK_SUCCESS);
		}
		MGM_CORE_INFO("Successfully allocated {} Command Buffers!", m_CommandBuffers.size());
	}

	void VulkanContext::Shutdown()
	{
		const Scope<GraphicsInstance>& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		VkDevice device = DynamicCast<VulkanDevice>(instance->GetDevice())->GetLogicalDevice();
		vkDeviceWaitIdle(device);
		vkFreeCommandBuffers(device, m_CommandPool, (u32)m_CommandBuffers.size(), m_CommandBuffers.data());
		vkDestroyCommandPool(device, m_CommandPool, nullptr);
	}

	void VulkanContext::BeginFrame()
	{

	}

	void VulkanContext::EndFrame()
	{

	}

	void VulkanContext::SetViewport(u32 x, u32 y, u32 width, u32 height)
	{
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<f32>(width);
		viewport.height = static_cast<f32>(height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_ActiveCommandBuffer, 0, 1, &viewport);
	}

	void VulkanContext::SetScissor(i32 x, i32 y, u32 width, u32 height)
	{
		VkRect2D scissor{};
		scissor.offset = { x, y };
		scissor.extent = { width, height };
		vkCmdSetScissor(m_ActiveCommandBuffer, 0, 1, &scissor);
	}

	void VulkanContext::BeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		// Begins a render pass
		m_ActiveRenderPass = renderPass;
	}

	void VulkanContext::EndRenderPass(const Ref<RenderPass>& renderPass)
	{
		// Ends a render pass
		MGM_CORE_ASSERT(m_ActiveRenderPass == renderPass,
			"Cannot end this render pass while other render pass is active!");
	}
}