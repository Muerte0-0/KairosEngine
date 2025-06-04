#pragma once
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "Image.h"
#include "Swapchain.h"

namespace Kairos
{
	class Frame
	{
	public:
		Frame(Swapchain& swapchainRef, VkDevice logicalDevice, std::vector<VkShaderEXT>& shaders, VkCommandBuffer commandBuffer, std::deque<std::function<void(VkDevice)>>& deletionQueue);

		void RecordCommandBuffer(uint32_t imageIndex, const Ref<class VertexArray>& vertexArray);

		VkCommandBuffer CommandBuffer;

		Swapchain& SwapchainRef;
		std::vector<VkShaderEXT>& Shaders;

		VkSemaphore ImageAquiredSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		VkFence RenderFinishedFence;

	private:
		void BuildRenderingInfo();
		void BuildColorAttachment(uint32_t imageIndex);
		void AnnoyingBoilerplateThatDynamicRenderingWasMeantToSpareUs(const class BufferLayout& layout);

		VkRenderingInfoKHR RenderingInfo = {};
		VkRenderingAttachmentInfoKHR ColorAttachment = {};

	};
}