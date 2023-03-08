#include "mgmpch.h"
#include "Renderer.h"
#include "RenderCommand.h"

namespace Magma
{
	void Renderer::Init()
	{
		RenderCommand::Init();
	}

	void Renderer::Shutdown()
	{
		RenderCommand::Shutdown();
	}
}