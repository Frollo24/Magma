#pragma once

#include "Magma/Core/Base.h"

namespace Magma
{
	enum class RenderAPI
	{
		None,
		Vulkan
	};

	class MAGMA_API RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		inline static RenderAPI GetAPI() { return s_API; }
		static Scope<RenderContext> Create();

	private:
		static RenderAPI s_API;
	};
}

