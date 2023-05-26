#include "mgmpch.h"
#include "VulkanDescriptors.h"

#include "VulkanDevice.h"
#include "VulkanUniformBuffer.h"
#include "VulkanTexture.h"

namespace Magma
{
	static VkDescriptorType GetVkDescriptorType(const DescriptorType& type)
	{
		switch (type)
		{
			case DescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case DescriptorType::ImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const DescriptorSetLayoutSpecification& spec, const Ref<RenderDevice>& device)
		: DescriptorSetLayout(spec), m_Device(DynamicCast<VulkanDevice>(device)->GetLogicalDevice())
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
		layoutBindings.reserve(m_Specification.Bindings.size());

		for (const auto& binding : m_Specification.Bindings)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = binding.Binding;
			layoutBinding.descriptorType = GetVkDescriptorType(binding.Type);
			layoutBinding.descriptorCount = 1;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

			layoutBindings.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<u32>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		VkResult result = vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_DescriptorLayout);
		MGM_CORE_VERIFY(result == VK_SUCCESS);
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_Device, m_DescriptorLayout, nullptr);
	}

	VulkanDescriptorPool::VulkanDescriptorPool(const Ref<RenderDevice>& device, const u32& maxFrames)
		: DescriptorPool(maxFrames), m_Device(DynamicCast<VulkanDevice>(device)->GetLogicalDevice())
	{
		std::vector<VkDescriptorPoolSize> pool_sizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * (u32)pool_sizes.size();
		pool_info.poolSizeCount = static_cast<u32>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();

		VkResult result = vkCreateDescriptorPool(m_Device, &pool_info, nullptr, &m_DescriptorPool);
		MGM_CORE_VERIFY(result == VK_SUCCESS);
	}

	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
	}
	
	VulkanDescriptorSet::VulkanDescriptorSet(const Ref<RenderDevice>& device, const Ref<DescriptorSetLayout>& layout, const Ref<DescriptorPool>& pool)
		: DescriptorSet(layout, pool),
		m_Device(DynamicCast<VulkanDevice>(device)->GetLogicalDevice()),
		m_DescriptorLayout(DynamicCast<VulkanDescriptorSetLayout>(m_Layout)->GetHandle()),
		m_DescriptorPool(DynamicCast<VulkanDescriptorPool>(m_Pool)->GetHandle())
	{
		InitDescriptorSets();
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		vkFreeDescriptorSets(m_Device, m_DescriptorPool, (u32)m_DescriptorSets.size(), m_DescriptorSets.data());

		for (auto& set : m_DescriptorSets)
			set = VK_NULL_HANDLE;

		m_DescriptorSets.clear();
	}

	void VulkanDescriptorSet::WriteUniformBuffer(const Ref<UniformBuffer>& uniformBuffer, u32 size)
	{
		Ref<VulkanUniformBuffer> vulkanBuffer = DynamicCast<VulkanUniformBuffer>(uniformBuffer);

		for (u32 i = 0; i < m_DescriptorSets.size(); i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = vulkanBuffer->GetHandle(i);
			bufferInfo.offset = 0;
			bufferInfo.range = size;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = vulkanBuffer->GetBinding();
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
		}
	}

	void VulkanDescriptorSet::WriteTexture2D(const Ref<Texture2D>& texture2D)
	{
		Ref<VulkanTexture2D> vulkanTexture = DynamicCast<VulkanTexture2D>(texture2D);

		for (const auto& descriptorSet : m_DescriptorSets)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = vulkanTexture->GetImageView();
			imageInfo.sampler = vulkanTexture->GetSampler();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = vulkanTexture->GetBinding();
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
		}
	}

	void VulkanDescriptorSet::WriteTextureCube(const Ref<TextureCube>& cubemap)
	{
		Ref<VulkanTextureCube> vulkanTexture = DynamicCast<VulkanTextureCube>(cubemap);

		for (const auto& descriptorSet : m_DescriptorSets)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = vulkanTexture->GetImageView();
			imageInfo.sampler = vulkanTexture->GetSampler();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = vulkanTexture->GetBinding();
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
		}
	}

	void VulkanDescriptorSet::WriteFramebufferTexture2D(const Ref<FramebufferTexture2D>& renderTarget)
	{
		Ref<VulkanFramebufferTexture2D> vulkanRenderTarget = DynamicCast<VulkanFramebufferTexture2D>(renderTarget);

		for (const auto& descriptorSet : m_DescriptorSets)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			imageInfo.imageView = vulkanRenderTarget->GetVkImageView();
			imageInfo.sampler = vulkanRenderTarget->GetVkSampler();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = vulkanRenderTarget->GetBinding();
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
		}
	}

	void VulkanDescriptorSet::InitDescriptorSets()
	{
		u32 maxFrames = m_Pool->GetMaxFrames();
		std::vector<VkDescriptorSetLayout> layouts(maxFrames, m_DescriptorLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = maxFrames;
		allocInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(maxFrames);
		VkResult result = vkAllocateDescriptorSets(m_Device, &allocInfo, m_DescriptorSets.data());
		MGM_CORE_VERIFY(result == VK_SUCCESS);
	}
}