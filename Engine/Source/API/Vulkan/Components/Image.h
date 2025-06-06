#pragma once

#include <vulkan/vulkan.h>

namespace Kairos
{
	VkImageView CreateImageView(VkDevice logicalDevice, VkImage image, VkFormat format);

	void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);
}