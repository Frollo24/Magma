#include "mgmpch.h"
#include "RenderContext.h"

namespace Magma
{
	RenderAPI RenderContext::s_API = RenderAPI::Vulkan;

	Scope<RenderContext> RenderContext::Create()
	{
		MGM_CORE_ASSERT(false, "Render context not yet implemented!");
		return nullptr;
	}
}