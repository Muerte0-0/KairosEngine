#pragma once

#include <vulkan/vulkan.h>

namespace Kairos
{
	VkCommandPool CreateCommandPool(VkDevice logicalDevice, uint32_t queueFamilyIndex, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue);
	VkCommandBuffer AllocateCommandBuffer(VkDevice logicalDevice, VkCommandPool commandPool);

	VkCommandBuffer BeginSingleTimeCommands(VkDevice logicalDevice, VkCommandPool commandPool);
	void EndSingleTimeCommands(VkDevice logicalDevice, VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue graphicsQueue);
}
