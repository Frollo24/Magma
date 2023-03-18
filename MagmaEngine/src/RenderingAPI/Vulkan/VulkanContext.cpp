#include "mgmpch.h"
#include "VulkanContext.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include "Magma/Core/Application.h"

namespace Magma
{
	struct FrameData
	{
		VkSemaphore PresentSemaphore;
		VkSemaphore RenderSemaphore;
		VkFence InFlightFence;
		VkResult ImageAcquireResult;
	};

	static std::vector<FrameData> s_FrameData{};

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

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		u32 framesInFlight = 2;
		s_FrameData.resize(framesInFlight);
		for (u32 i = 0; i < framesInFlight; i++)
		{
			VkResult result = vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &s_FrameData[i].PresentSemaphore);
			MGM_CORE_VERIFY(result == VK_SUCCESS);
			result = vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &s_FrameData[i].RenderSemaphore);
			MGM_CORE_VERIFY(result == VK_SUCCESS);
			result = vkCreateFence(logicalDevice, &fenceInfo, nullptr, &s_FrameData[i].InFlightFence);
			MGM_CORE_VERIFY(result == VK_SUCCESS);
		}
	}

	void VulkanContext::Shutdown()
	{
		const Scope<GraphicsInstance>& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		VkDevice device = DynamicCast<VulkanDevice>(instance->GetDevice())->GetLogicalDevice();
		vkDeviceWaitIdle(device);

		for (u32 i = 0; i < s_FrameData.size(); i++)
		{
			vkDestroySemaphore(device, s_FrameData[i].PresentSemaphore, nullptr);
			vkDestroySemaphore(device, s_FrameData[i].RenderSemaphore, nullptr);
			vkDestroyFence(device, s_FrameData[i].InFlightFence, nullptr);
		}
		s_FrameData.clear();

		vkFreeCommandBuffers(device, m_CommandPool, (u32)m_CommandBuffers.size(), m_CommandBuffers.data());
		vkDestroyCommandPool(device, m_CommandPool, nullptr);

		m_ActiveRenderPass = nullptr;
	}

	void VulkanContext::BeginFrame()
	{
		// Acquires a valid image from the swapchain
		const Scope<GraphicsInstance>& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		VkDevice device = DynamicCast<VulkanDevice>(instance->GetDevice())->GetLogicalDevice();
		const Ref<VulkanSwapchain>& swapchain = DynamicCast<VulkanSwapchain>(instance->GetSwapchain());

		vkWaitForFences(device, 1, &s_FrameData[m_CurrentFrame].InFlightFence, VK_TRUE, UINT64_MAX);
		u32 imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapchain->GetHandle(), UINT64_MAX,
			s_FrameData[m_CurrentFrame].PresentSemaphore, VK_NULL_HANDLE, &imageIndex);
		swapchain->SetImageIndex(imageIndex);

		s_FrameData[m_CurrentFrame].ImageAcquireResult = result;
		if (s_FrameData[m_CurrentFrame].ImageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;

		// Begins a frame recording
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		result = vkBeginCommandBuffer(m_CommandBuffers[imageIndex], &beginInfo);
		MGM_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin recording command buffer!");

		m_ActiveCommandBuffer = m_CommandBuffers[imageIndex];
	}

	void VulkanContext::EndFrame()
	{
		// Ends a frame recording
		if (s_FrameData[m_CurrentFrame].ImageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;

		VkResult result = vkEndCommandBuffer(m_ActiveCommandBuffer);
		MGM_CORE_ASSERT(result == VK_SUCCESS, "Failed to record command buffer");

		// Submit the command buffer to a queue
		const Scope<GraphicsInstance>& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		VkDevice device = DynamicCast<VulkanDevice>(instance->GetDevice())->GetLogicalDevice();
		VkQueue graphicsQueue = DynamicCast<VulkanDevice>(instance->GetDevice())->GetQueueHandles().GraphicsQueue;

		u32 imageIndex = DynamicCast<VulkanSwapchain>(instance->GetSwapchain())->GetImageIndex();
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { s_FrameData[m_CurrentFrame].PresentSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { s_FrameData[m_CurrentFrame].RenderSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device, 1, &s_FrameData[m_CurrentFrame].InFlightFence);
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, s_FrameData[m_CurrentFrame].InFlightFence);
	}

	void VulkanContext::Present()
	{
		const Scope<GraphicsInstance>& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const Ref<VulkanDevice>& device = DynamicCast<VulkanDevice>(instance->GetDevice());
		const Ref<VulkanSwapchain>& swapchain = DynamicCast<VulkanSwapchain>(instance->GetSwapchain());

		u32 imageIndex = swapchain->GetImageIndex();
		VkQueue graphicsQueue = device->GetQueueHandles().GraphicsQueue;
		VkQueue presentQueue = device->GetQueueHandles().PresentQueue;

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		VkSemaphore waitSemaphores[] = { s_FrameData[m_CurrentFrame].RenderSemaphore };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemaphores;

		VkSwapchainKHR swapChains[] = { swapchain->GetHandle() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(presentQueue ? presentQueue : graphicsQueue, &presentInfo);

		MGM_CORE_TRACE("ImageIndex: {} // Current Frame: {} // Total Frames: {}", imageIndex, m_CurrentFrame, m_TotalFrames);
		m_TotalFrames++;
		m_CurrentFrame = m_TotalFrames % s_FrameData.size();
	}

	void VulkanContext::SetViewport(u32 x, u32 y, u32 width, u32 height)
	{
		if (s_FrameData[m_CurrentFrame].ImageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;

		VkViewport viewport{};
		viewport.x = static_cast<f32>(x);
		viewport.y = static_cast<f32>(y);
		viewport.width = static_cast<f32>(width);
		viewport.height = static_cast<f32>(height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_ActiveCommandBuffer, 0, 1, &viewport);
	}

	void VulkanContext::SetScissor(i32 x, i32 y, u32 width, u32 height)
	{
		if (s_FrameData[m_CurrentFrame].ImageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;

		VkRect2D scissor{};
		scissor.offset = { x, y };
		scissor.extent = { width, height };
		vkCmdSetScissor(m_ActiveCommandBuffer, 0, 1, &scissor);
	}

	void VulkanContext::BeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		if (s_FrameData[m_CurrentFrame].ImageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;

		// Begins a render pass
		const Scope<GraphicsInstance>& instance = Application::Instance().GetWindow().GetGraphicsInstance();
		const Ref<VulkanSwapchain>& swapchain = DynamicCast<VulkanSwapchain>(instance->GetSwapchain());
		const Ref<VulkanRenderPass>& pass = DynamicCast<VulkanRenderPass>(renderPass);
		u32 imageIndex = swapchain->GetImageIndex();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pass->GetHandle();

		const bool& isSwapchainTarget = renderPass->GetSpecification().IsSwapchainTarget;
		renderPassInfo.framebuffer = isSwapchainTarget ?
			swapchain->GetFramebuffer(imageIndex)->GetHandle() :
			pass->GetFramebuffer()->GetHandle();

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = isSwapchainTarget ?
			swapchain->GetExtent() :
			pass->GetFramebuffer()->GetExtent();

		const ClearValues& clearValues = renderPass->GetSpecification().ClearValues;
		const auto& color = clearValues.Color;

		std::vector<VkClearValue> clearValue(renderPass->GetSpecification().Attachments.size());
		for (size_t i = 0; i < clearValue.size(); i++)
		{
			switch (renderPass->GetSpecification().Attachments[i])
			{
				case AttachmentFormat::RGBA8:
					clearValue[i].color = { color.r, color.g, color.b, color.a };
					break;
				default:
					break;
			}
		}

		renderPassInfo.clearValueCount = static_cast<u32>(clearValue.size());
		renderPassInfo.pClearValues = clearValue.data();

		vkCmdBeginRenderPass(m_ActiveCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		m_ActiveRenderPass = renderPass;
	}

	void VulkanContext::EndRenderPass(const Ref<RenderPass>& renderPass)
	{
		if (s_FrameData[m_CurrentFrame].ImageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;

		// Ends a render pass
		MGM_CORE_ASSERT(m_ActiveRenderPass == renderPass,
			"Cannot end this render pass while other render pass is active!");

		if (s_FrameData[m_CurrentFrame].ImageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;

		vkCmdEndRenderPass(m_ActiveCommandBuffer);
	}
}