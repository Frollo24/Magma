#pragma once

#include "Magma/Core/Base.h"

#include <vulkan/vulkan.h>

namespace Magma
{
	class ValidationLayers
	{
	public:
		static bool CheckValidationLayerSupport();

		static void SetupDebugMessenger(VkInstance instance);
		static void DestroyDebugMessenger(VkInstance instance);
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);

		inline static const std::vector<const char*>& GetLayers() { return c_ValidationLayers; }
		inline static const bool& Enabled() { return c_Enabled; }

	private:
#ifdef MGM_DEBUG
		static const bool c_Enabled = true;
#else
		static const bool c_Enabled = false;
#endif // MGM_DEBUG

		static const std::vector<const char*> c_ValidationLayers;

		static VkDebugUtilsMessengerEXT s_DebugMessenger;
		static VkDebugUtilsMessengerCreateInfoEXT s_CreateInfo;

	};
}

