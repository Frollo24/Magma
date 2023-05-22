#include "mgmpch.h"
#include "UniformBuffer.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanUniformBuffer.h"

namespace Magma
{
	Ref<UniformBuffer> UniformBuffer::Create(const Ref<RenderDevice>& device, u32 size, u32 binding, u32 maxFrames)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanUniformBuffer>(device, size, binding, maxFrames);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}
