#include "mgmpch.h"
#include "Shader.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanShader.h"

namespace Magma
{
	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanShader>(filepath);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}