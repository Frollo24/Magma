#include "mgmpch.h"
#include "Framebuffer.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanFramebuffer.h"

namespace Magma
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec, const Ref<RenderDevice>& device, const Ref<RenderPass>& renderPass)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanFramebuffer>(spec, device, renderPass);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}