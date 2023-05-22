#include "mgmpch.h"
#include "Descriptors.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanDescriptors.h"

namespace Magma
{
	Ref<DescriptorSetLayout> DescriptorSetLayout::Create(const DescriptorSetLayoutSpecification& spec, const Ref<RenderDevice>& device)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanDescriptorSetLayout>(spec, device);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}

	Ref<DescriptorPool> DescriptorPool::Create(const Ref<RenderDevice>& device, const u32 maxFrames)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanDescriptorPool>(device, maxFrames);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}

	Ref<DescriptorSet> DescriptorSet::Create(const Ref<RenderDevice>& device, const Ref<DescriptorSetLayout>& layout, const Ref<DescriptorPool>& pool)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanDescriptorSet>(device, layout, pool);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}
}