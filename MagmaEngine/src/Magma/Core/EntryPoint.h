#pragma once
#include "Magma.h"

extern Magma::Application* Magma::CreateApplication(); // To be defined by client

namespace Magma
{
	int Main(int argc, char** argv)
	{
		auto app = Magma::CreateApplication();
		app->Run();
		delete app;

		return 0;
	}
}

int main(int argc, char** argv)
{
	return Magma::Main(argc, argv);
}
