#pragma once

#include <vulkan/vulkan.h>

namespace Kairos
{
	VkSemaphore MakeSemaphore(VkDevice logicalDevice, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue);
	VkFence MakeFence(VkDevice logicalDevice, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue);
}