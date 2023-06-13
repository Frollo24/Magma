#include "mgmpch.h"
#include "ValidationLayers.h"

namespace Magma
{
	const std::vector<const char*> ValidationLayers::c_ValidationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	VkDebugUtilsMessengerEXT ValidationLayers::s_DebugMessenger{};
	VkDebugUtilsMessengerCreateInfoEXT ValidationLayers::s_CreateInfo{};

	VkResult CreateDebugUtilsMessengerEXT(VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
			func(instance, debugMessenger, pAllocator);
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData)
	{
		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:   MGM_CORE_TRACE("[VALIDATION LAYER - VERBOSE]: {0}\n", pCallbackData->pMessage);   break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:      MGM_CORE_INFO("[VALIDATION LAYER - INFO]: {0}\n", pCallbackData->pMessage);       break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:   MGM_CORE_WARN("[VALIDATION LAYER - WARNING]: {0}\n", pCallbackData->pMessage);    break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:     MGM_CORE_ERROR("[VALIDATION LAYER - ERROR]: {0}\n", pCallbackData->pMessage);     break;
			default: break;
		}

		return VK_FALSE;
	}

	bool ValidationLayers::CheckValidationLayerSupport()
	{
		u32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayerProps(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayerProps.data());

		for (const char* layerName : c_ValidationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProps : availableLayerProps)
				if (layerFound = (strcmp(layerName, layerProps.layerName) == 0))
					break;

			if (!layerFound)
				return false;
		}

		return true;
	}

	void ValidationLayers::SetupDebugMessenger(VkInstance instance)
	{
		if (!c_Enabled) return;

		PopulateDebugMessengerCreateInfo(s_CreateInfo);
		VkResult result = CreateDebugUtilsMessengerEXT(instance, &s_CreateInfo, nullptr, &s_DebugMessenger);
		MGM_CORE_VERIFY(result == VK_SUCCESS);
	}

	void ValidationLayers::DestroyDebugMessenger(VkInstance instance)
	{
		if (c_Enabled)
			DestroyDebugUtilsMessengerEXT(instance, s_DebugMessenger, nullptr);
	}

	void ValidationLayers::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo)
	{
		debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = DebugCallback;
	}
}