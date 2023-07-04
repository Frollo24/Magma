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
		CPU
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

		static Ref<RenderDevice> Create(GraphicsInstance& instance, RenderSurface& surface, const PhysicalDeviceRequirements& requirements);
		static Ref<RenderDevice> Create(const Scope<GraphicsInstance>& instance, const Scope<RenderSurface>& surface, const PhysicalDeviceRequirements& requirements);
	};
}

