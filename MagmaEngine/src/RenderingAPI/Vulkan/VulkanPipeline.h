#pragma once
#include "Magma/Renderer/Pipeline.h"

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const PipelineSpecification& spec, const Ref<RenderDevice>& device, const Ref<RenderPass>& renderPass);
		virtual ~VulkanPipeline();

		inline Ref<RenderPass> GetRenderPass() const override { return m_RenderPass; }
		inline const VkPipeline& GetHandle() const { return m_Pipeline; }
		inline const VkPipelineLayout& GetLayout() const { return m_PipelineLayout; }
		inline const VkPipelineBindPoint& GetBindPoint() const { return m_PipelineBindPoint; }

	private:
		void CreateGraphicsPipeline(const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages);

	private:
		Ref<VulkanRenderPass> m_RenderPass = nullptr;

		VkDevice m_Device = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipelineBindPoint m_PipelineBindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;
	};
}


