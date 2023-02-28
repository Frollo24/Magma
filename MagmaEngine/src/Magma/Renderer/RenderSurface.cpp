#include "mgmpch.h"
#include "RenderSurface.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanSurface.h"

namespace Magma
{
	Scope<RenderSurface> RenderSurface::Create(const Scope<GraphicsInstance>& instance)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateScope<VulkanSurface>(instance);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}