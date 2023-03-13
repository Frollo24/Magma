#include "mgmpch.h"
#include "VulkanRenderPass.h"

#include "VulkanDevice.h"

namespace Magma
{
	static VkFormat AttachmentFormatToVkFormat(const AttachmentFormat& format)
	{
		switch (format)
		{
			case AttachmentFormat::RGBA8: return VK_FORMAT_R8G8B8A8_SRGB;
			default: return VK_FORMAT_UNDEFINED;
		}
	}

	static VkAttachmentDescription CreateColorAttachment(AttachmentFormat format, bool isSwapchainTarget, bool clearValue, u32 numSamples,
		VkAttachmentReference& reference, uint32_t attachment)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format =
			isSwapchainTarget ? VK_FORMAT_B8G8R8A8_SRGB : AttachmentFormatToVkFormat(format);
		colorAttachment.samples = (VkSampleCountFlagBits)numSamples;
		colorAttachment.loadOp = clearValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.finalLayout = isSwapchainTarget ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.initialLayout = clearValue ? VK_IMAGE_LAYOUT_UNDEFINED : colorAttachment.finalLayout;

		reference.attachment = attachment;
		reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		return colorAttachment;
	}

	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec, const Ref<RenderDevice>& device)
		: RenderPass(spec), m_Device(DynamicCast<VulkanDevice>(device)->GetLogicalDevice())
	{
		u32 attachmentIndex = 0;
		std::vector<VkAttachmentDescription> totalAttachmentsDescs{};
		std::vector<VkAttachmentReference> totalAttachmentsRefs{};
		for (const auto& attachment : m_Specification.Attachments)
		{
			VkAttachmentDescription attachmentDesc{};
			VkAttachmentReference attachmentRef{};

			switch (attachment)
			{
				case AttachmentFormat::RGBA8:
					attachmentDesc = CreateColorAttachment(attachment, m_Specification.IsSwapchainTarget, (u8)(m_Specification.ClearValues.ClearFlags & ClearFlags::Color),
						m_Specification.Samples, attachmentRef, attachmentIndex);
					break;
				default:
					break;
			}

			totalAttachmentsDescs.push_back(attachmentDesc);
			totalAttachmentsRefs.push_back(attachmentRef);
			attachmentIndex++;
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<u32>(totalAttachmentsRefs.size());
		subpass.pColorAttachments = totalAttachmentsRefs.data();

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = 0;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachmentIndex;
		renderPassInfo.pAttachments = totalAttachmentsDescs.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass);
		MGM_CORE_VERIFY(result == VK_SUCCESS);

		MGM_CORE_INFO("Successfully created Vulkan Render Pass!");
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
	}
}