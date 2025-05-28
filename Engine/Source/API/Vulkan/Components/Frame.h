#pragma once
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

namespace Kairos
{
	class Frame
	{
	public:
		Frame(VkImage image, VkDevice logicalDevice, VkFormat swapchainFormat, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue);

		void SetCommandBuffer(VkCommandBuffer newCommandBuffer, std::vector<VkShaderEXT>& shaders, VkExtent2D frameSize);

		VkImage Image;

		VkImageView ImageView;

		VkCommandBuffer CommandBuffer;

		VkSemaphore ImageAquiredSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		VkFence RenderFinishedFence;

	private:
		void BuildRenderingInfo(VkExtent2D frameSize);
		void BuildColorAttachment(VkClearValue clearColor = VkClearValue({ 0.1f, 0.1f, 0.1f, 1.0f }));

		VkRenderingInfoKHR RenderingInfo = {};
		VkRenderingAttachmentInfoKHR ColorAttachment = {};

	};
}