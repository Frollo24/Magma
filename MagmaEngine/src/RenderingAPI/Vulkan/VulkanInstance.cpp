#include "mgmpch.h"
#include "VulkanInstance.h"

#include <GLFW/glfw3.h>

namespace Magma
{
	VulkanInstance::VulkanInstance(void* window)
		: m_WindowHandle(window)
	{
	}

	VulkanInstance::~VulkanInstance()
	{
		vkDestroyInstance(m_Instance, nullptr);
	}

	void VulkanInstance::Init()
	{
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
		createInfo.enabledLayerCount = 0;

		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
		MGM_CORE_VERIFY(result == VK_SUCCESS);
		CheckAvailableExtensions();
	}

	void VulkanInstance::SetVSync(bool enabled)
	{
		// Change the presentation mode of the swapchain
	}

	void VulkanInstance::PresentFrame()
	{
		// Present an image from the swapchain to the window
	}

	std::vector<const char*> VulkanInstance::GetRequiredExtensions()
	{
		u32 extensionCount;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionCount);

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