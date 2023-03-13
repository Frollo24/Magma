#include "mgmpch.h"
#include "RenderPass.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanRenderPass.h"

namespace Magma
{
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec, const Ref<RenderDevice>& device)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanRenderPass>(spec, device);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}