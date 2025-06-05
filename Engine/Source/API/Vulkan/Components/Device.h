#pragma once

#include "vulkan/vulkan.h"

namespace Kairos
{
	struct QueueFamilyIndices
	{
		uint32_t GraphicsIndex;
		uint32_t PresentIndex;
		uint32_t ComputeIndex;
		bool HasCompute;
	};

	bool Supports(const VkPhysicalDevice physicalDevice, const char** requestedExtensions, const uint32_t requestedExtensionCount);
	bool IsSuitable(const VkPhysicalDevice physicalDevice);

	VkPhysicalDevice ChoosePhysicalDevice(VkInstance& instance);

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	uint32_t FindQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueueFlags queueType);

	VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue);
}