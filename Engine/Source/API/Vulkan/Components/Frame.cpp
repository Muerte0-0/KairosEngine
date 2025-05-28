#include "kepch.h"

#include "Frame.h"
#include "Image.h"

#include "Engine/Renderer/Renderer.h"

#include "volk.h"

namespace Kairos
{
	Frame::Frame(VkImage image, VkDevice logicalDevice, VkFormat swapchainFormat, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue)
	{
		ImageView = CreateImageView(logicalDevice, image, swapchainFormat);

		VkImageView imageViewHandle = ImageView;

		deviceDeletionQueue.push_back([imageViewHandle](VkDevice device)
			{
				vkDestroyImageView(device, imageViewHandle, nullptr);
			});
	}

	void Frame::SetCommandBuffer(VkCommandBuffer newCommandBuffer, std::vector<VkShaderEXT>& shaders, VkExtent2D frameSize)
	{
		CommandBuffer = newCommandBuffer;

		BuildColorAttachment();
		BuildRenderingInfo(frameSize);

		VkCommandBufferBeginInfo beginInfo = {};

		vkBeginCommandBuffer(CommandBuffer, &beginInfo);

		TransitionImageLayout(CommandBuffer, Image,
			VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			VkAccessFlagBits::VK_ACCESS_NONE, VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		vkCmdBeginRendering(CommandBuffer, &RenderingInfo);

		VkShaderStageFlagBits stages[2] = {
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
			VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT
		};

		for (auto shader : shaders)
		vkCmdBindShadersEXT(CommandBuffer, shaders.size(), stages, &shader);

		vkCmdDraw(CommandBuffer, 3, 1, 0, 0);

		vkCmdEndRendering(CommandBuffer);

		TransitionImageLayout(CommandBuffer, Image,
			VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VkAccessFlagBits::VK_ACCESS_NONE,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

		vkEndCommandBuffer(CommandBuffer);
	}

	void Frame::BuildRenderingInfo(VkExtent2D frameSize)
	{
		RenderingInfo.flags = VkRenderingFlagsKHR();
		RenderingInfo.renderArea = VkRect2D({ 0,0 }, frameSize);
		RenderingInfo.layerCount = 1;
		RenderingInfo.viewMask = 0;
		RenderingInfo.colorAttachmentCount = 1;
		RenderingInfo.pColorAttachments = &ColorAttachment;
	}

	void Frame::BuildColorAttachment(VkClearValue clearColor)
	{
		ColorAttachment.imageView = ImageView;
		ColorAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		ColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		ColorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		ColorAttachment.clearValue = clearColor;
	}
}