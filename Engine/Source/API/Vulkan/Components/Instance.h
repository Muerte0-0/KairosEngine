#pragma once
#include "vulkan/vulkan.h"

#include <GLFW/glfw3.h>

namespace Kairos
{
	bool SupportedByInstance(const char** extensionNames, int extensionCount, const char** layerNames, int layerCount);

	VkInstance CreateInstance(const char* applicationName, std::deque<std::function<void(VkInstance)>>& instanceDeletionQueue);
	VkSurfaceKHR CreateSurface(VkInstance instance,GLFWwindow* windowHandle, std::deque<std::function<void(VkInstance)>>& instanceDeletionQueue);

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	VkDebugUtilsMessengerEXT MakeDebugMessenger(VkInstance instance, std::deque<std::function<void(VkInstance)>> instanceDeletionQueue);
}