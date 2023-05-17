#include "mgmpch.h"
#include "VertexBuffer.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanVertexBuffer.h"

namespace Magma
{
	Ref<VertexBuffer> VertexBuffer::Create(const Ref<RenderDevice>& device, u32 size, const void* data)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanVertexBuffer>(device, size, data);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}