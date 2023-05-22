#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderDevice.h"

namespace Magma
{
	class MAGMA_API UniformBuffer
	{
	public:
		virtual ~UniformBuffer() = default;

		static Ref<UniformBuffer> Create(const Ref<RenderDevice>& device, u32 size, u32 binding, u32 maxFrames);

		inline const u32& GetMaxFrames() { return m_MaxFrames; }
		inline const u32& GetBinding() { return m_Binding; }

		virtual void SetCommonDataForAllFrames(const void* data, u32 size, u32 offset = 0) = 0;
		virtual void SetData(const void* data, u32 size, u32 offset = 0) = 0;
		virtual void SetDataToFrame(u32 frame, const void* data, u32 size, u32 offset = 0) = 0;

	protected:
		explicit UniformBuffer(u32 binding, u32 maxFrames) : m_Binding(binding), m_MaxFrames(maxFrames) {}
		u32 m_Binding = 0;
		u32 m_MaxFrames = 0;
	};
}

