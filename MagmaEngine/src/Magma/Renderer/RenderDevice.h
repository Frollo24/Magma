#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Renderer/GraphicsInstance.h"

namespace Magma
{
	enum class PhysicalDeviceType
	{
		Other,
		IntegratedGPU,
		DedicatedGPU,
		VirtualGPU,
		CPU,
		DontCare,
		Unknown
	};

	struct PhysicalDeviceRequirements
	{
		PhysicalDeviceType DeviceType = PhysicalDeviceType::DedicatedGPU;
		bool SupportsGeometryShaders = false;
		bool SupportsTesselationShaders = false;
		bool SupportsMSAA = false;
		bool SupportsAnisotropy = false;
		bool SupportsRaytracing = false;
	};

	class RenderDevice
	{
	public:
		virtual ~RenderDevice() = default;

		static Ref<RenderDevice> Create(const RawPointer<GraphicsInstance>& instance, const RawPointer<RenderSurface>& surface, const PhysicalDeviceRequirements& requirements);
	};
}

