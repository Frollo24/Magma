#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderDevice.h"

namespace Magma
{
	class MAGMA_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		static Ref<VertexBuffer> Create(const Ref<RenderDevice>& device, u32 size, const void* data);
	};
}

