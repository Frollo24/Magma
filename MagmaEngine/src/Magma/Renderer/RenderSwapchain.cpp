#include "mgmpch.h"
#include "RenderSwapchain.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanSwapchain.h"

namespace Magma
{
	Ref<RenderSwapchain> RenderSwapchain::Create(const Ref<RenderDevice>& device, const RawPointer<RenderSurface>& surface, void* window)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanSwapchain>(device, surface, window);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}