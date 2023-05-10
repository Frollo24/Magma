#pragma once

namespace Magma
{
	enum class ShaderType
	{
		None = 0,
		Vertex,
		Fragment,
		Count
	};

	using ShaderBinaryMap = std::unordered_map<ShaderType, std::vector<u32>>;
	using ShaderStreamMap = std::unordered_map<ShaderType, std::stringstream>;

	class MAGMA_API Shader
	{
	public:
		virtual ~Shader() = default;

		static Ref<Shader> Create(const std::string& filepath);

	protected:
		virtual ShaderBinaryMap Compile(const ShaderStreamMap& shaderSources) const = 0;
	};
}

