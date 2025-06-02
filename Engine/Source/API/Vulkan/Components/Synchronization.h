#pragma once
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

namespace Kairos
{
	VkSemaphore MakeSemaphore(VkDevice logicalDevice, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue);
	VkFence MakeFence(VkDevice logicalDevice, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue);
}