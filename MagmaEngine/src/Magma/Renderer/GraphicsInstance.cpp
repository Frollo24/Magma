#include "mgmpch.h"
#include "GraphicsInstance.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanInstance.h"

namespace Magma
{
	Scope<GraphicsInstance> GraphicsInstance::Create(void* windowHandle)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateScope<VulkanInstance>(windowHandle);
		}
	}
}