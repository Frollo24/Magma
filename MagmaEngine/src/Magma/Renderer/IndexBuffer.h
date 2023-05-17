#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderDevice.h"

namespace Magma
{
	enum class IndexSize
	{
		U16, U32
	};

	static u64 SizeofIndex(const IndexSize& size)
	{
		switch (size)
		{
			case IndexSize::U16: return sizeof(u16);
			case IndexSize::U32: return sizeof(u32);
			default:
				return 0;
		}
	}

	class MAGMA_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		static Ref<IndexBuffer> Create(const Ref<RenderDevice>& device, u32 count, const void* indices, const IndexSize& indexSize = IndexSize::U32);

		inline const IndexSize& GetIndexSize() { return m_IndexSize; }

	protected:
		explicit IndexBuffer(const IndexSize& indexSize) : m_IndexSize(indexSize) {}
		IndexSize m_IndexSize = IndexSize::U32;
	};
}

