#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/VertexBuffer.h"
#include "Magma/Renderer/IndexBuffer.h"

#include <glm/glm.hpp>

namespace Magma
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		bool operator==(const Vertex& other) const {
			return Position == other.Position
				&& Normal == other.Normal
				&& TexCoord == other.TexCoord
				&& Tangent == other.Tangent
				&& Bitangent == other.Bitangent;
		}
	};

	enum class ModelBuilder
	{
		TinyObj,
		Assimp // TODO: include Assimp as model loader
	};

	class MAGMA_API Model
	{
	public:
		Model(const std::string& filepath, const Ref<RenderDevice>& device, const ModelBuilder& builder = ModelBuilder::TinyObj);
		virtual ~Model();

		void Render();

	private:
		void BuildTinyObjModel();
		void BuildAssimpModel();

	private:
		Ref<RenderDevice> m_Device = nullptr;
		Ref<VertexBuffer> m_VertexBuffer = nullptr;
		Ref<IndexBuffer> m_IndexBuffer = nullptr;

		std::string m_Filepath;
		u32 m_VertexCount = 0;
		u32 m_IndexCount = 0;
	};
}

