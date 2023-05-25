#include "mgmpch.h"
#include "Material.h"

namespace Magma
{
	Material::Material(const glm::vec4& color, const float& metallic, const float& roughness)
		: m_Color(color), m_Metallic(metallic), m_Roughness(roughness)
	{
	}

	void Material::SetTexture(const Ref<Texture2D>& texture, const TextureType& type)
	{
		texture->SetBinding((u32)type);
		m_DescriptorSet->WriteTexture2D(texture);
		m_MaterialTextures[(u32)type] = texture;
	}
}