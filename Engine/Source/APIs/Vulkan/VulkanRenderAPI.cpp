#include "kepch.h"
#include "VulkanRenderAPI.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Engine
{
	const std::vector<char const*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	constexpr bool enableValidationLayers = false;
#else
	constexpr bool enableValidationLayers = true;
#endif
	
	void VulkanRenderAPI::Init(void* windowHandle)
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface(windowHandle);
	}

	void VulkanRenderAPI::BeginFrame()
	{
	}

	void VulkanRenderAPI::EndFrame()
	{
	}

	void VulkanRenderAPI::SetClearColor(float r, float g, float b, float a)
	{
	}

	void VulkanRenderAPI::Clear()
	{
	}

	void VulkanRenderAPI::CreateInstance()
	{
		constexpr vk::ApplicationInfo appInfo{
			"Engine",
			VK_MAKE_VERSION(1, 0, 0),
			"No Engine",
			VK_MAKE_VERSION(1, 0, 0),
			vk::ApiVersion14
			};
		
		// Get the required layers
		std::vector<char const*> requiredLayers;
		if (enableValidationLayers)
		{
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());
		}
		
		// Check if the required layers are supported by the Vulkan implementation.
		auto layerProperties = m_Context.enumerateInstanceLayerProperties();
		auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
			[&layerProperties](auto const& requiredLayer) {
				return std::ranges::none_of(layerProperties,
					[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
			});
		if (unsupportedLayerIt != requiredLayers.end())
		{
			throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
		}

		// Get the required extensions.
		auto requiredExtensions = GetRequiredInstanceExtensions();

		// Check if the required extensions are supported by the Vulkan implementation.
		auto extensionProperties = m_Context.enumerateInstanceExtensionProperties();
		auto unsupportedPropertyIt =
			std::ranges::find_if(requiredExtensions,
				[&extensionProperties](auto const& requiredExtension) {
					return std::ranges::none_of(extensionProperties,
						[requiredExtension](auto const& extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
				});
		if (unsupportedPropertyIt != requiredExtensions.end())
		{
			throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
		}

		vk::InstanceCreateInfo createInfo;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
		createInfo.ppEnabledLayerNames = requiredLayers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		m_Instance = vk::raii::Instance(m_Context, createInfo);
	}
	
	std::vector<const char*> VulkanRenderAPI::GetRequiredInstanceExtensions() const
	{
		uint32_t glfwExtensionCount = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
			extensions.push_back(vk::EXTDebugUtilsExtensionName);

		return extensions;
	}

	void VulkanRenderAPI::SetupDebugMessenger()
	{
		if (!enableValidationLayers) return;

		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT;
		debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags;
		debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags;
		debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &DebugCallback;

		try
		{
			m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
		}
		catch (vk::SystemError& err)
		{
			cout << "Debug messenger not available. Validation layers may not be enabled." << err.what() << endl;
		}
	}

	void VulkanRenderAPI::CreateSurface(void* windowHandle)
	{
		VkSurfaceKHR surface;
		
		if (glfwCreateWindowSurface(*m_Instance, static_cast<GLFWwindow*>(windowHandle), nullptr, &surface) != 0) {
			throw std::runtime_error("failed to create window surface!");
		}
		m_Surface = vk::raii::SurfaceKHR(m_Instance, surface);
	}
}
