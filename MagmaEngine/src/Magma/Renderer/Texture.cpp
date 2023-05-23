#include "mgmpch.h"
#include "Texture.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanTexture.h"

namespace Magma
{
	Ref<Texture2D> Texture2D::Create(const Ref<RenderDevice>& device, const std::string& filepath)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanTexture2D>(device, filepath);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}