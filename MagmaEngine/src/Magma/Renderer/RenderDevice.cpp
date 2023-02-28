#include "mgmpch.h"
#include "RenderDevice.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanDevice.h"


namespace Magma
{
	Ref<RenderDevice> RenderDevice::Create(GraphicsInstance& instance, const Scope<RenderSurface>& surface, const PhysicalDeviceRequirements& requirements)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanDevice>(instance, surface, requirements);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}

	Ref<RenderDevice> RenderDevice::Create(const Scope<GraphicsInstance>& instance, const Scope<RenderSurface>& surface, const PhysicalDeviceRequirements& requirements)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanDevice>(instance, surface, requirements);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}