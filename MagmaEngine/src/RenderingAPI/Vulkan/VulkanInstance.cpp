#include "mgmpch.h"
#include "VulkanInstance.h"
#include "ValidationLayers.h"

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanSurface.h"
#include <GLFW/glfw3.h>

namespace Magma
{
	VulkanInstance::VulkanInstance(void* window)
		: m_WindowHandle(window)
	{
		MGM_CORE_ASSERT(window, "Window handle is null!");
		if (ValidationLayers::Enabled())
		{
			MGM_CORE_VERIFY(ValidationLayers::CheckValidationLayerSupport());
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_API_VERSION_1_1;
		appInfo.pEngineName = "Magma Engine";

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		if (ValidationLayers::Enabled())
		{
			auto& layers = ValidationLayers::GetLayers();
			createInfo.enabledLayerCount = static_cast<u32>(layers.size());
			createInfo.ppEnabledLayerNames = layers.data();

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
			ValidationLayers::PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;
		}

		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
		MGM_CORE_VERIFY(result == VK_SUCCESS);
		CheckAvailableExtensions();

		if (ValidationLayers::Enabled())
			ValidationLayers::SetupDebugMessenger(m_Instance);
	}

	VulkanInstance::~VulkanInstance()
	{
		if (ValidationLayers::Enabled())
			ValidationLayers::DestroyDebugMessenger(m_Instance);

		vkDestroyInstance(m_Instance, nullptr);
	}

	void VulkanInstance::Init(const Scope<RenderSurface>& surface)
	{
		PhysicalDeviceRequirements requirements{};
		requirements.DeviceType = PhysicalDeviceType::DedicatedGPU;
		requirements.SupportsGeometryShaders = true;
		requirements.SupportsTesselationShaders = true;
		requirements.SupportsAnisotropy = true;
		// requirements.SupportsRaytracing = true;
		m_Device = RenderDevice::Create(*this, *surface.get(), requirements);
		m_Swapchain = RenderSwapchain::Create(m_Device, *surface.get(), m_WindowHandle);
	}

	void VulkanInstance::Shutdown()
	{
		m_Swapchain = nullptr;
		m_Device = nullptr;
	}

	void VulkanInstance::SetVSync(bool enabled)
	{
		// Change the presentation mode of the swapchain
	}

	void VulkanInstance::PresentFrame()
	{
		// Present an image from the swapchain to the window
		m_Swapchain->PresentFrame();
	}

	std::vector<const char*> VulkanInstance::GetRequiredExtensions()
	{
		u32 extensionCount;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionCount);

		if (ValidationLayers::Enabled()) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void VulkanInstance::CheckAvailableExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		MGM_CORE_INFO("{0} extensions supported", extensionCount);
		MGM_CORE_INFO("Available extensions:");

		// Not using logger for console readability
		for (const auto& extension : extensions) {
			if (std::string(extension.extensionName).find("VK_NV") != std::string::npos)
				std::cout << "\u001b[1;32m" << '\t' << extension.extensionName << "\u001b[0m" << '\n';
			else if (std::string(extension.extensionName).find("VK_AMD") != std::string::npos)
				std::cout << "\u001b[31m" << '\t' << extension.extensionName << "\u001b[0m" << '\n';
			else if (std::string(extension.extensionName).find("VK_ARM") != std::string::npos)
				std::cout << "\u001b[36m" << '\t' << extension.extensionName << "\u001b[0m" << '\n';
			else if (std::string(extension.extensionName).find("VK_EXT") != std::string::npos)
				std::cout << "\u001b[1;30m" << '\t' << extension.extensionName << "\u001b[0m" << '\n';
			else
				std::cout << '\t' << extension.extensionName << '\n';
		}
	}
}