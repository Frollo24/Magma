#pragma once

#include "Magma/Core/Base.h"

namespace Magma
{
	class MAGMA_API Application
	{
	public:
		Application();
		virtual ~Application();

		inline static Application& Instance() { return *s_Instance; }

		void Run();

	private:
		static Application* s_Instance;
	};

	// To be defined by client
	Application* CreateApplication();
}
