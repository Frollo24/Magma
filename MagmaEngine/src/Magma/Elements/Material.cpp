#include "mgmpch.h"
#include "Material.h"

#include "Magma/Renderer/Renderer.h"

namespace Magma
{
	Material::Material(const glm::vec4& color, const float& metallic, const float& roughness)
		: m_Color(color), m_Metallic(metallic), m_Roughness(roughness)
	{
		m_DescriptorSet = DescriptorSet::Create(Renderer::GetDevice(), Renderer::GetMaterialDescriptorSetLayout(), Renderer::GetDescriptorPool());

		for (sz i = 0; i < m_MaterialTextures.size(); i++)
		{
			SetTexture(Texture2D::Create(Renderer::GetDevice(), "assets/textures/no-texture.png"), (TextureType)i);
		}
	}

	void Material::SetTexture(const Ref<Texture2D>& texture, const TextureType& type)
	{
		texture->SetBinding((u32)type);
		m_DescriptorSet->WriteTexture2D(texture);
		m_MaterialTextures[(u32)type] = texture;
	}
}