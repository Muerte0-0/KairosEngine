#pragma once
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "API/Vulkan/VulkanContext.h"

namespace Kairos
{
	void CreateCommandPool(VulkanContext* vctx);
	void CreateCommandBuffers(VulkanContext* vctx);
	void CreateSyncObjects(VulkanContext* vctx);

	VkCommandBuffer BeginSingleTimeCommands(VulkanContext* vctx);
	void EndSingleTimeCommands(VulkanContext* vctx, VkCommandBuffer commandBuffer);

	void RenderFrame(VulkanContext* vctx);

	void RecordCommandBuffer(VulkanContext* vctx, VkCommandBuffer commandBuffer, uint32_t imageIndex);
}
