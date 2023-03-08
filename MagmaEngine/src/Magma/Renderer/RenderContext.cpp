#include "mgmpch.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanContext.h"

namespace Magma
{
	RenderAPI RenderContext::s_API = RenderAPI::Vulkan;

	Scope<RenderContext> RenderContext::Create()
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateScope<VulkanContext>();
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}