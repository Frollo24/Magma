#pragma once

namespace Magma
{
	class SubsystemManager
	{
	public:
		static void InitSubsystems();
		static void Shutdown();

	private:
		static void InitDepthPrepassSubsystem();
		static void InitSimpleSubsystem();
		static void InitDefaultForwardSubsystem();
		static void InitDefaultGBufferSubsystem();
		static void InitDefaultDeferredSubsystem();

		static void InitUniformBuffers();
	};
}

