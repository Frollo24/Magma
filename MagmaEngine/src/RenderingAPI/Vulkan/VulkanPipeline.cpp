#include "mgmpch.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"

namespace Magma
{
	static VkFormat ShaderTypeToVulkanType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:    return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2:   return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3:   return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4:   return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderDataType::Mat3:     return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Mat4:     return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderDataType::Int:      return VK_FORMAT_R32_SINT;
			case ShaderDataType::Int2:     return VK_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:     return VK_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:     return VK_FORMAT_R32G32B32A32_SINT;
			case ShaderDataType::Bool:     return VK_FORMAT_R32_SINT;
			default:
				MGM_CORE_ASSERT(false, "Unknown shader data type!");
				return VK_FORMAT_MAX_ENUM;
		}
	}

	static VkVertexInputBindingDescription GetBindingDescription(const InputElementsLayout& layout)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = layout.GetStride();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions(const InputElementsLayout& layout)
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		attributeDescriptions.resize(layout.GetElements().size());

		u32 location = 0;
		for (const auto& element : layout.GetElements())
		{
			VkVertexInputAttributeDescription attrDescription{};
			attrDescription.binding = 0;
			attrDescription.location = location;
			attrDescription.format = ShaderTypeToVulkanType(element.Type);
			attrDescription.offset = element.Offset;

			attributeDescriptions[location] = attrDescription;
			location++;
		}

		return attributeDescriptions;
	}

	VulkanPipeline::VulkanPipeline(const PipelineSpecification& spec, const Ref<RenderDevice>& device, const Ref<RenderPass>& renderPass)
		: Pipeline(spec), m_RenderPass(DynamicCast<VulkanRenderPass>(renderPass)),
		m_Device(DynamicCast<VulkanDevice>(device)->GetLogicalDevice())
	{
		const auto& shader = DynamicCast<VulkanShader>(m_Specification.Shader);
		std::unordered_map<ShaderType, VkShaderModule> modules{};
		shader->CreateShaderModules(m_Device, modules);

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
		shaderStages.reserve(modules.size());
		for (const auto& [shaderType, shaderModule] : modules)
		{
			VkPipelineShaderStageCreateInfo createInfo{};
			shader->PopulateShaderStageCreateInfo(createInfo, shaderType, shaderModule);
			shaderStages.push_back(createInfo);
		}

		switch (m_Specification.PipelineType)
		{
			case PipelineType::Graphics:
				CreateGraphicsPipeline(shaderStages);
				break;
		}

		shader->DestroyShaderModules(m_Device, modules);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
		vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
	}

	void VulkanPipeline::CreateGraphicsPipeline(const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages)
	{
		m_PipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		auto vertexBindingDescription = GetBindingDescription(m_Specification.InputElementsLayout);
		auto vertexAttributeDescriptions = GetAttributeDescriptions(m_Specification.InputElementsLayout);
		sz inputElementsSize = m_Specification.InputElementsLayout.GetElements().size();

		VkPipelineVertexInputStateCreateInfo vertexInput{};
		vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInput.vertexBindingDescriptionCount = inputElementsSize ? 1 : 0;
		vertexInput.pVertexBindingDescriptions = &vertexBindingDescription;
		vertexInput.vertexAttributeDescriptionCount = static_cast<u32>(vertexAttributeDescriptions.size());
		vertexInput.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr; // Viewports are specified within command buffers
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr; // Scissors are specified within command buffers

		VkPipelineRasterizationStateCreateInfo rasterization{};
		rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization.rasterizerDiscardEnable = VK_FALSE;
		rasterization.depthClampEnable = VK_FALSE;
		rasterization.polygonMode = (VkPolygonMode)m_Specification.PipelinePolygonState.PolygonMode;
		rasterization.lineWidth = 1.0f;
		rasterization.frontFace = (VkFrontFace)m_Specification.PipelinePolygonState.FrontFace;
		rasterization.cullMode = (VkCullModeFlags)m_Specification.PipelinePolygonState.CullMode;
		rasterization.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = (VkSampleCountFlagBits)m_RenderPass->GetSpecification().Samples;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = m_Specification.PipelineBlendState.ColorWriteMask;
		colorBlendAttachment.blendEnable = m_Specification.PipelineBlendState.BlendEnable;
		colorBlendAttachment.srcColorBlendFactor = (VkBlendFactor)m_Specification.PipelineBlendState.ColorEquation.SrcFactor;
		colorBlendAttachment.dstColorBlendFactor = (VkBlendFactor)m_Specification.PipelineBlendState.ColorEquation.DstFactor;
		colorBlendAttachment.colorBlendOp = (VkBlendOp)m_Specification.PipelineBlendState.ColorEquation.Operation;
		colorBlendAttachment.srcAlphaBlendFactor = (VkBlendFactor)m_Specification.PipelineBlendState.AlphaEquation.SrcFactor;
		colorBlendAttachment.dstAlphaBlendFactor = (VkBlendFactor)m_Specification.PipelineBlendState.AlphaEquation.DstFactor;
		colorBlendAttachment.alphaBlendOp = (VkBlendOp)m_Specification.PipelineBlendState.AlphaEquation.Operation;

		// TODO: add support for multiple color blend attachments
		VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
		colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendingInfo.logicOpEnable = VK_FALSE;
		colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendingInfo.attachmentCount = 1;
		colorBlendingInfo.pAttachments = &colorBlendAttachment;
		colorBlendingInfo.blendConstants[0] = m_Specification.PipelineBlendState.ConstantColor.R;
		colorBlendingInfo.blendConstants[1] = m_Specification.PipelineBlendState.ConstantColor.G;
		colorBlendingInfo.blendConstants[2] = m_Specification.PipelineBlendState.ConstantColor.B;
		colorBlendingInfo.blendConstants[3] = m_Specification.PipelineBlendState.ConstantColor.A;

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilInfo.depthTestEnable = m_Specification.PipelineDepthState.DepthTest;
		depthStencilInfo.depthWriteEnable = m_Specification.PipelineDepthState.DepthWrite;
		depthStencilInfo.depthCompareOp = (VkCompareOp)m_Specification.PipelineDepthState.DepthFunc;
		depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilInfo.stencilTestEnable = VK_FALSE;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VkResult result = vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
		MGM_CORE_VERIFY(result == VK_SUCCESS);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = static_cast<u32>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.pVertexInputState = &vertexInput;
		pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pRasterizationState = &rasterization;
		pipelineCreateInfo.pMultisampleState = &multisampling;
		pipelineCreateInfo.pColorBlendState = &colorBlendingInfo;
		pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;

		pipelineCreateInfo.renderPass = m_RenderPass->GetHandle();
		pipelineCreateInfo.subpass = 0;
		pipelineCreateInfo.layout = m_PipelineLayout;

		result = vkCreateGraphicsPipelines(m_Device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_Pipeline);
		MGM_CORE_VERIFY(result == VK_SUCCESS);
	}
}
