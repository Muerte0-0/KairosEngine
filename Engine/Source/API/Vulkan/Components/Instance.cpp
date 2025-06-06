#include "kepch.h"

#include "Instance.h"
#include "API/Vulkan/VulkanUtils.h"

#include "volk.h"

namespace Kairos
{
	bool SupportedByInstance(const char** extensionNames, uint32_t extensionCount, const char** layerNames, uint32_t layerCount)
	{
		uint32_t supportedExtensionsCount;
		std::vector<VkExtensionProperties> supportedExtensions;
		vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionsCount, nullptr);
		supportedExtensions.resize(supportedExtensionsCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionsCount, supportedExtensions.data());

		bool found;
		for (int i = 0; i < extensionCount; ++i)
		{
			const char* extension = extensionNames[i];
			found = false;
			for (VkExtensionProperties supportedExtension : supportedExtensions)
			{
				if (strcmp(extension, supportedExtension.extensionName) == 0)
				{
					found = true;
					break;
				}
			}
			if (!found)
				return false;
		}

		uint32_t supportedLayersCount;
		std::vector<VkLayerProperties> supportedLayers;
		vkEnumerateInstanceLayerProperties(&supportedLayersCount, nullptr);
		supportedLayers.resize(supportedLayersCount);
		vkEnumerateInstanceLayerProperties(&supportedExtensionsCount, supportedLayers.data());


		for (int i = 0; i < layerCount; ++i)
		{
			const char* layer = layerNames[i];
			found = false;
			for (VkLayerProperties supportedLayer : supportedLayers)
			{
				if (strcmp(layer, supportedLayer.layerName) == 0)
				{
					found = true;
					break;
				}
			}
			if (!found)
				return false;
		}

		return true;
	}

	VkInstance CreateInstance(const char* applicationName, std::deque<std::function<void(VkInstance)>>& instanceDeletionQueue)
	{
		uint32_t version;
		vkEnumerateInstanceVersion(&version);
		version &= ~(0xFFFU);

		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = applicationName;
		appInfo.apiVersion = version;
		appInfo.pEngineName = "Kairos Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pNext = NULL;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> enabledExtensions;

		for (uint32_t i = 0; i < glfwExtensionCount; ++i)
			enabledExtensions.push_back(glfwExtensions[i]);

#ifdef KE_DEBUG
		enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // KE_DEBUG

		std::vector<const char*> enabledLayers;

#ifdef KE_DEBUG
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif // KE_DEBUG
		
		if (!SupportedByInstance(enabledExtensions.data(), (uint32_t)enabledExtensions.size(), enabledLayers.data(), (uint32_t)enabledLayers.size()))
			return nullptr;

		VkInstanceCreateInfo info = {};

		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.flags = VkInstanceCreateFlags();
		info.pApplicationInfo = &appInfo;

		info.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		info.ppEnabledExtensionNames = enabledExtensions.data();

		info.enabledLayerCount = (uint32_t)enabledLayers.size();
		info.ppEnabledLayerNames = enabledLayers.data();

		VkInstance instance;

		VK_CHECK(vkCreateInstance(&info, nullptr, &instance));

		instanceDeletionQueue.push_back([](VkInstance instance)
			{
				vkDestroyInstance(instance, nullptr);
				KE_CORE_INFO("Deleted Instance!");
			});

		return instance;
	}

	VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* windowHandle, std::deque<std::function<void(VkInstance)>>& instanceDeletionQueue)
	{
		VkSurfaceKHR raw_surface = VK_NULL_HANDLE;

		VK_CHECK(glfwCreateWindowSurface(instance, windowHandle, nullptr, &raw_surface));

		instanceDeletionQueue.push_back([raw_surface](VkInstance instance)
			{
				vkDestroySurfaceKHR(instance, raw_surface, nullptr);
				KE_CORE_INFO("Deleted Surface!");
			});
		
		return raw_surface;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkDebugUtilsMessengerEXT CreateDebugMessenger(VkInstance instance, std::deque<std::function<void(VkInstance)>> instanceDeletionQueue)
	{
#ifdef KE_DEBUG
		KE_CORE_INFO("Creating Debug Messenger!");

		VkDebugUtilsMessengerCreateInfoEXT dci;
		dci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		dci.flags = VkDebugUtilsMessengerCreateFlagsEXT();
		dci.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

		dci.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

		dci.pfnUserCallback = debugCallback;

		VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
		vkCreateDebugUtilsMessengerEXT(instance, &dci, nullptr, &messenger);

		instanceDeletionQueue.push_back([messenger](VkInstance instance)
			{
				vkDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
				KE_CORE_INFO("Deleted Debug Messender!");
			});

		return messenger;

#endif // KE_DEBUG

		return nullptr;
	}
}