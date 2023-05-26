#pragma once
#include "Magma/Renderer/Descriptors.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	class VulkanDescriptorSetLayout : public DescriptorSetLayout
	{
	public:
		VulkanDescriptorSetLayout(const DescriptorSetLayoutSpecification& spec, const Ref<RenderDevice>& device);
		virtual ~VulkanDescriptorSetLayout();

		inline const VkDescriptorSetLayout& GetHandle() const { return m_DescriptorLayout; }

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorLayout = VK_NULL_HANDLE;
	};

	class VulkanDescriptorPool : public DescriptorPool
	{
	public:
		VulkanDescriptorPool(const Ref<RenderDevice>& device, const u32& maxFrames = 2);
		virtual ~VulkanDescriptorPool();

		inline const VkDescriptorPool& GetHandle() const { return m_DescriptorPool; }

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

		u32 m_MaxFrames = 0;
	};

	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		VulkanDescriptorSet(const Ref<RenderDevice>& device, const Ref<DescriptorSetLayout>& layout, const Ref<DescriptorPool>& pool);
		virtual ~VulkanDescriptorSet();

		inline const VkDescriptorSet& GetHandle(u32 frame) const { return m_DescriptorSets[frame]; }
		inline const VkDescriptorSetLayout& GetLayout() const { return m_DescriptorLayout; }
		inline const VkDescriptorPool& GetPool() const { return m_DescriptorPool; }

		virtual void WriteUniformBuffer(const Ref<UniformBuffer>& uniformBuffer, u32 size) override;
		virtual void WriteTexture2D(const Ref<Texture2D>& texture) override;
		virtual void WriteTextureCube(const Ref<TextureCube>& cubemap) override;
		virtual void WriteFramebufferTexture2D(const Ref<FramebufferTexture2D>& renderTarget) override;

	private:
		void InitDescriptorSets();

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorLayout = VK_NULL_HANDLE;
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_DescriptorSets{};
	};
}