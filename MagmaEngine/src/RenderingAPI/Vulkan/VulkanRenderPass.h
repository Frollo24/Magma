#pragma once
#include "Magma/Renderer/RenderPass.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& spec, const Ref<RenderDevice>& device);
		virtual ~VulkanRenderPass();

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	};
}

