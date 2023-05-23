#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/RenderDevice.h"

namespace Magma
{
	struct TextureDimensions
	{
		union
		{
			struct
			{
				u32 width;
				u32 height;
				u32 depth;
			};
			struct
			{
				u32 x;
				u32 y;
				u32 z;
			};
		};
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual void SetData(const void* data, u32 size) = 0;

		inline const TextureDimensions& GetDimensions() const { return m_Dimensions; }
		inline void SetDimensions(const TextureDimensions& dimensions) { m_Dimensions = dimensions; }

		inline const u32& GetBinding() const { return m_Binding; }
		inline void SetBinding(const u32 binding) { m_Binding = binding; }

	protected:
		TextureDimensions m_Dimensions = { 1, 1, 1 };
		u32 m_Binding = 0;
	};

	class MAGMA_API Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const Ref<RenderDevice>& device, const std::string& filepath);
	};
}

