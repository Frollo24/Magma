#include "mgmpch.h"
#include "IndexBuffer.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanIndexBuffer.h"

namespace Magma
{
	Ref<IndexBuffer> IndexBuffer::Create(const Ref<RenderDevice>& device, u32 count, const void* data, const IndexSize& indexSize)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanIndexBuffer>(device, count, data, indexSize);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}