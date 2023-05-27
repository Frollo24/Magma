#pragma once

namespace Magma
{
	class SubsystemManager
	{
	public:
		static void InitSubsystems();
		static void Update();
		static void Shutdown();

	private:
		static void InitDepthPrepassSubsystem();
		static void InitSimpleSubsystem();
		static void InitDefaultForwardSubsystem();
		static void InitDefaultGBufferSubsystem();
		static void InitDefaultDeferredSubsystem();
		static void InitSimpleDeferredSubsystem();
		static void InitSSAOKernelSubsystem();
		static void InitSSAOBlurSubsystem();
		static void InitSkyboxSubsystem();

		static void InitDescriptorLayouts();
		static void InitUniformBuffers();
		static void InitTextures();
	};
}

