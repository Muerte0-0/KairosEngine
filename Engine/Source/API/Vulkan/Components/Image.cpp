#include "kepch.h"

#include "Image.h"

#include "volk.h"

namespace Kairos
{
	VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format)
	{
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.image = image;
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView view;
        vkCreateImageView(device, &viewInfo, nullptr, &view);
        return view;
	}

	void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
	{
		VkImageSubresourceRange access;
		access.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		access.baseMipLevel = 0;
		access.levelCount = 1;
		access.baseArrayLayer = 0;
		access.layerCount = 1;

		VkImageMemoryBarrier2 barrier;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = access;

		VkPipelineStageFlags sourceStage, destinationStage;

		barrier.srcAccessMask = srcAccessMask;
		barrier.dstAccessMask = dstAccessMask;

		//vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, VkDependencyFlags(),);

		VkDependencyInfo depInfo = {};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.dependencyFlags = VkDependencyFlags();
		depInfo.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(commandBuffer, &depInfo);
	}
}