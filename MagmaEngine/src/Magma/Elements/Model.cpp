#include "mgmpch.h"
#include "Model.h"

#include "Magma/Renderer/RenderCommand.h"

#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std
{
	template<> struct hash<Magma::Vertex> {
		size_t operator()(Magma::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.Position) ^
				(hash<glm::vec3>()(vertex.Normal) << 1)) >> 1)
				^
				((hash<glm::vec2>()(vertex.TexCoord) >> 1) ^
					((hash<glm::vec3>()(vertex.Tangent) >> 2) ^
						(hash<glm::vec3>()(vertex.Bitangent) << 2)) << 1);
		}
	};
}

namespace Magma
{
	template<typename U>
	struct TinyObjBuilder
	{
		std::vector<Vertex> Vertices{};
		std::vector<U> Indices{};

		void Build(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, const u32 indexCount);
	};

	template<typename U>
	inline void TinyObjBuilder<U>::Build(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, const u32 indexCount)
	{
		std::unordered_map<Vertex, U> uniqueVertices;

		Indices.reserve(indexCount);

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.Position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.Normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
				glm::normalize(vertex.Normal);

				if (attrib.texcoords.size()) {
					vertex.TexCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<U>(Vertices.size());
					Vertices.push_back(vertex);
				}

				Indices.push_back(uniqueVertices[vertex]);
			}
		}

		auto tangents = std::vector<glm::vec3>(Vertices.size(), glm::vec3(0.0f));
		auto bitangents = std::vector<glm::vec3>(Vertices.size(), glm::vec3(0.0f));

		for (sz i = 0; i < Indices.size(); i += 3)
		{
			int i0 = Indices[i + 0];
			int i1 = Indices[i + 1];
			int i2 = Indices[i + 2];

			const glm::vec3& p0 = Vertices[i0].Position;
			const glm::vec3& p1 = Vertices[i1].Position;
			const glm::vec3& p2 = Vertices[i2].Position;

			const glm::vec2& w0 = Vertices[i0].TexCoord;
			const glm::vec2& w1 = Vertices[i1].TexCoord;
			const glm::vec2& w2 = Vertices[i2].TexCoord;

			glm::vec3 e1 = p1 - p0;
			glm::vec3 e2 = p2 - p0;

			float x1 = w1.x - w0.x;
			float x2 = w2.x - w0.x;
			float y1 = w1.y - w0.y;
			float y2 = w2.y - w0.y;

			float r = 1.0f / (x1 * y2 - x2 * y1);
			glm::vec3 t = (e1 * y2 - e2 * y1) * r;
			glm::vec3 b = (e2 * x1 - e1 * x2) * r;

			tangents[i0] += t;
			tangents[i1] += t;
			tangents[i2] += t;
			bitangents[i0] += b;
			bitangents[i1] += b;
			bitangents[i2] += b;
		}

		i32 leftHanded = 0;
		i32 rightHanded = 0;
		for (sz i = 0; i < Vertices.size(); i++)
		{
			const glm::vec3& t = tangents[i];
			const glm::vec3& b = bitangents[i];
			const glm::vec3& n = Vertices[i].Normal;

			const glm::vec3& reject_t = (t - glm::dot(t, n) * n);
			Vertices[i].Tangent = glm::normalize(reject_t);

			float determinant = (glm::dot(glm::cross(t, b), n) > 0.0f) ? 1.0f : -1.0f;
			if (determinant > 0.0f) rightHanded++;
			else leftHanded++;

			Vertices[i].Bitangent = determinant * glm::cross(Vertices[i].Normal, Vertices[i].Tangent);
		}

		MGM_CORE_TRACE("Left Handed: {} // Right Handed: {}", leftHanded, rightHanded);
	}

	Model::Model(const std::string& filepath, const Ref<RenderDevice>& device, const ModelBuilder& builder)
		: m_Device(device), m_Filepath(filepath)
	{
		switch (builder)
		{
			case ModelBuilder::TinyObj:
				BuildTinyObjModel();
				break;
			case ModelBuilder::Assimp:
				BuildAssimpModel();
				break;
			default:
				break;
		}
	}

	Model::~Model()
	{
		m_VertexBuffer = nullptr;
		m_IndexBuffer = nullptr;
		m_Device = nullptr;
	}

	void Model::Render()
	{
		// TODO: handle no indexed models

		RenderCommand::BindVertexBuffer(m_VertexBuffer);
		RenderCommand::BindIndexBuffer(m_IndexBuffer);
		RenderCommand::DrawIndices(m_IndexCount, 1, 0, 0);
	}

	void Model::BuildTinyObjModel()
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;

		bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, m_Filepath.c_str());
		MGM_CORE_ASSERT(result, err);

		for (const auto& shape : shapes)
			m_IndexCount += static_cast<u32>(shape.mesh.indices.size());

		if (m_IndexCount <= UINT16_MAX)
		{
			TinyObjBuilder<u16> builder;
			builder.Build(shapes, attrib, m_IndexCount);

			m_IndexBuffer = IndexBuffer::Create(m_Device, m_IndexCount, builder.Indices.data(), IndexSize::U16);
			m_VertexCount = static_cast<u32>(builder.Vertices.size());
			m_VertexBuffer = VertexBuffer::Create(m_Device, m_VertexCount * (u32)sizeof(Vertex), builder.Vertices.data());
		}
		else
		{
			TinyObjBuilder<u32> builder;
			builder.Build(shapes, attrib, m_IndexCount);

			m_IndexBuffer = IndexBuffer::Create(m_Device, m_IndexCount, builder.Indices.data(), IndexSize::U32);
			m_VertexCount = static_cast<u32>(builder.Vertices.size());
			m_VertexBuffer = VertexBuffer::Create(m_Device, m_VertexCount * (u32)sizeof(Vertex), builder.Vertices.data());
		}
	}

	void Model::BuildAssimpModel()
	{
		// TODO: include Assimp as model loader
	}
}