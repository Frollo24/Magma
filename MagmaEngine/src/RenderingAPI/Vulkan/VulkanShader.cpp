#include "mgmpch.h"
#include "VulkanShader.h"

#include <shaderc/shaderc.hpp>

namespace Utils
{
	static std::string ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				MGM_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			MGM_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	class VulkanIncluder : public shaderc::CompileOptions::IncluderInterface
	{
	public:
		// Handles shaderc_include_resolver_fn callbacks.
		virtual shaderc_include_result* GetInclude(const char* requested_source,
			shaderc_include_type type,
			const char* requesting_source,
			size_t include_depth) override
		{
			auto path = std::string("assets/shaders/") + requested_source;
			auto relpath = std::string("") + requested_source;
			auto test = ReadFile(path);

			MGM_CORE_TRACE("Picked {0} as include", requested_source);
			auto container = new std::array<std::string, 2>;
			(*container)[0] = std::string(requested_source);
			(*container)[1] = test;

			auto data = new shaderc_include_result;
			data->user_data = container;
			data->source_name = (*container)[0].data();
			data->source_name_length = (*container)[0].size();
			data->content = (*container)[1].data();
			data->content_length = (*container)[1].size();

			return data;
		}

		// Handles shaderc_include_result_release_fn callbacks.
		virtual void ReleaseInclude(shaderc_include_result* data)
		{
			delete static_cast<std::array<std::string, 2>*>(data->user_data);
			delete data;
			MGM_CORE_TRACE("Released previous include!");
		}

		virtual ~VulkanIncluder() override = default;
	};
}

namespace Magma
{
	static VkShaderStageFlagBits ShaderTypeToVkStageFlag(ShaderType shaderType)
	{
		switch (shaderType)
		{
			case ShaderType::None:			return (VkShaderStageFlagBits)0;
			case ShaderType::Vertex:		return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderType::Fragment:		return VK_SHADER_STAGE_FRAGMENT_BIT;

			default:
				return (VkShaderStageFlagBits)0;
		}
	}

	static shaderc_shader_kind ShaderTypeToShaderCKind(ShaderType shaderType)
	{
		switch (shaderType)
		{
			case ShaderType::None:			return (shaderc_shader_kind)0;
			case ShaderType::Vertex:		return shaderc_glsl_vertex_shader;
			case ShaderType::Fragment:		return shaderc_glsl_fragment_shader;

			default:
				return (shaderc_shader_kind)0;
		}
	}

	static std::string ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				MGM_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			MGM_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	static std::unordered_map<ShaderType, std::stringstream> PreProcess(const std::string& source)
	{
		std::unordered_map<ShaderType, std::stringstream> shaderSources;
		const char* typeToken = "#shader";

		std::stringstream stream(source);
		std::string line;

		ShaderType type = ShaderType::None;
		while (getline(stream, line))
		{
			if (line.find(typeToken) != std::string::npos)
			{
				if (line.find("vertex") != std::string::npos)
					type = ShaderType::Vertex;
				else if (line.find("fragment") != std::string::npos)
					type = ShaderType::Fragment;
			}
			else
			{
				shaderSources[type] << line << '\n';
			}
		}

		return shaderSources;
	}

	ShaderBinaryMap VulkanShader::Compile(const ShaderStreamMap& shaderSources) const
	{
		ShaderBinaryMap shaderSpvSources{};

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);
		options.SetIncluder(CreateScope<Utils::VulkanIncluder>());

		for (const auto& [shaderType, source] : shaderSources)
		{
			shaderc::PreprocessedSourceCompilationResult preprocessed = compiler.PreprocessGlsl(source.str(), ShaderTypeToShaderCKind(shaderType), m_FilePath.c_str(), options);
			auto preprocessResult = preprocessed.GetCompilationStatus();
			MGM_CORE_ASSERT(preprocessResult == shaderc_compilation_status_success, preprocessed.GetErrorMessage().c_str());
			std::string preprocessedString = { preprocessed.cbegin(), preprocessed.cend() };

			shaderc::SpvCompilationResult spvModule = compiler.CompileGlslToSpv(preprocessedString, ShaderTypeToShaderCKind(shaderType), m_FilePath.c_str(), options);
			auto compileResult = spvModule.GetCompilationStatus();
			MGM_CORE_ASSERT(compileResult == shaderc_compilation_status_success, spvModule.GetErrorMessage().c_str());
			const auto& data = std::vector<u32>(spvModule.cbegin(), spvModule.cend());
			shaderSpvSources[shaderType] = data;
		}

		return shaderSpvSources;
	}

	VulkanShader::VulkanShader(const std::string& filepath)
		: m_FilePath(filepath)
	{
		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);
		m_ShaderSpvSources = Compile(shaderSources);
	}

	void VulkanShader::CreateShaderModules(VkDevice device, ShaderModuleMap& shaderModules) const
	{
		for (const auto& [shaderType, spvSource] : m_ShaderSpvSources)
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = spvSource.size() * 4; // Size must be specified in bytes
			createInfo.pCode = spvSource.data();

			VkShaderModule shaderModule = VK_NULL_HANDLE;
			shaderModules[shaderType] = shaderModule;

			VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModules[shaderType]);
			MGM_CORE_VERIFY(result == VK_SUCCESS);
		}
	}

	void VulkanShader::PopulateShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo, ShaderType shaderType, VkShaderModule shaderModule) const
	{
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.stage = ShaderTypeToVkStageFlag(shaderType);
		createInfo.module = shaderModule;
		createInfo.pName = "main";
	}

	void VulkanShader::DestroyShaderModules(VkDevice device, const ShaderModuleMap& shaderModules) const
	{
		for (const auto& [shaderType, shaderModule] : shaderModules)
		{
			vkDestroyShaderModule(device, shaderModule, nullptr);
		}
	}
}