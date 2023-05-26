#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/Descriptors.h"

#include <glm/glm.hpp>

namespace Magma
{
	enum class TextureType
	{
		Albedo = 0,
		Normal = 1,
		Metallic = 2,
		Roughness = 3,
		Emmisive = 4,
		Count
	};

	class MAGMA_API Material
	{
	public:
		Material(const glm::vec4& color = glm::vec4(1.0f), const float& metallic = 0.5f, const float& roughness = 1.0f);
		~Material();

		inline const glm::vec4& GetColor() const { return m_Color; }
		inline void SetColor(const glm::vec4& color) { m_Color = color; }

		inline const float& GetMetallic() const { return m_Metallic; }
		inline void SetMetallic(const float& metallic) { m_Metallic = metallic; }

		inline const float& GetRoughness() const { return m_Roughness; }
		inline void SetRoughness(const float& roughness) { m_Roughness = roughness; }

		inline const Ref<Texture2D>& GetTexture(const TextureType& type) { return m_MaterialTextures[(sz)type]; }
		void SetTexture(const Ref<Texture2D>& texture, const TextureType& type);

		inline const Ref<DescriptorSet>& GetDescriptorSet() { return m_DescriptorSet; }

	private:
		glm::vec4 m_Color;
		float m_Metallic;
		float m_Roughness;
		Ref<DescriptorSet> m_DescriptorSet = nullptr;

		std::array<Ref<Texture2D>, (sz)TextureType::Count> m_MaterialTextures{};
	};
}

