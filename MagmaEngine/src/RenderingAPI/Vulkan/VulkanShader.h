#pragma once
#include "Magma/Renderer/Shader.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	using ShaderModuleMap = std::unordered_map<ShaderType, VkShaderModule>;

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& filepath);
		virtual ~VulkanShader() = default;

		void CreateShaderModules(VkDevice device, ShaderModuleMap& shaderModules) const;
		void PopulateShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo, ShaderType shaderType, VkShaderModule shaderModule) const;
		void DestroyShaderModules(VkDevice device, const ShaderModuleMap& shaderModules) const;

	protected:
		virtual ShaderBinaryMap Compile(const ShaderStreamMap& shaderSources) const override;

	private:
		std::string m_FilePath;
		std::unordered_map<ShaderType, std::vector<u32>> m_ShaderSpvSources{};
	};
}
