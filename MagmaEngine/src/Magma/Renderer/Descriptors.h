#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderDevice.h"
#include "Magma/Renderer/UniformBuffer.h"
#include "Magma/Renderer/Texture.h"

namespace Magma
{
	enum class DescriptorType
	{
		UniformBuffer,
		ImageSampler
	};

	struct DescriptorBinding
	{
		DescriptorType Type;
		u32 Binding = 0;
	};

	struct DescriptorSetLayoutSpecification
	{
		std::vector<DescriptorBinding> Bindings{};
	};

	class MAGMA_API DescriptorSetLayout
	{
	public:
		virtual ~DescriptorSetLayout() = default;

		static Ref<DescriptorSetLayout> Create(const DescriptorSetLayoutSpecification& spec, const Ref<RenderDevice>& device);

		inline const DescriptorSetLayoutSpecification& GetSpecification() const { return m_Specification; }

	protected:
		explicit DescriptorSetLayout(const DescriptorSetLayoutSpecification& spec) : m_Specification(spec) {}
		DescriptorSetLayoutSpecification m_Specification;
	};

	class MAGMA_API DescriptorPool
	{
	public:
		virtual ~DescriptorPool() = default;

		static Ref<DescriptorPool> Create(const Ref<RenderDevice>& device, const u32 maxFrames = 2);

		inline const u32& GetMaxFrames() const { return m_MaxFrames; }

	protected:
		explicit DescriptorPool(const u32& maxFrames) : m_MaxFrames(maxFrames) {}
		u32 m_MaxFrames = 0;
	};

	class MAGMA_API DescriptorSet
	{
	public:
		virtual ~DescriptorSet() = default;

		static Ref<DescriptorSet> Create(const Ref<RenderDevice>& device, const Ref<DescriptorSetLayout>& layout, const Ref<DescriptorPool>& pool);

		virtual void WriteUniformBuffer(const Ref<UniformBuffer>& uniformBuffer, u32 size) = 0;
		virtual void WriteTexture2D(const Ref<Texture2D>& texture) = 0;
		virtual void WriteTextureCube(const Ref<TextureCube>& cubemap) = 0;
		virtual void WriteFramebufferTexture2D(const Ref<FramebufferTexture2D>& renderTarget) = 0;

	protected:
		explicit DescriptorSet(const Ref<DescriptorSetLayout>& layout, const Ref<DescriptorPool>& pool) : m_Layout(layout), m_Pool(pool) {}
		Ref<DescriptorSetLayout> m_Layout = nullptr;
		Ref<DescriptorPool> m_Pool = nullptr;
	};
}
