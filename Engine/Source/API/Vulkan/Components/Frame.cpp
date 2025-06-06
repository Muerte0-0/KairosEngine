#include "kepch.h"

#include "Frame.h"
#include "Synchronization.h"
#include "Swapchain.h"

#include "API/Vulkan/VulkanUtils.h"
#include "API/Vulkan/VulkanFramebuffer.h"

#include "Engine/Core/Application.h"
#include "Engine/Renderer/Renderer.h"

#include "volk.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

namespace Kairos
{
	Frame::Frame(Swapchain& swapchainRef, VkDevice logicalDevice, VkCommandBuffer commandBuffer, std::deque<std::function<void(VkDevice)>>& deletionQueue)
		: SwapchainRef(swapchainRef), CommandBuffer(commandBuffer)
	{
		ImageAquiredSemaphore = MakeSemaphore(logicalDevice, deletionQueue);
		RenderFinishedSemaphore = MakeSemaphore(logicalDevice, deletionQueue);
		RenderFinishedFence = MakeFence(logicalDevice, deletionQueue);
	}

	void Frame::RecordCommandBuffer(uint32_t imageIndex, const Ref<class VertexArray>& vertexArray)
	{
		vkResetCommandBuffer(CommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		BuildColorAttachment(imageIndex);
		BuildRenderingInfo();

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		vkBeginCommandBuffer(CommandBuffer, &beginInfo);

		Ref<VulkanFramebuffer> frameBuffer = std::dynamic_pointer_cast<VulkanFramebuffer>(Renderer::GetFramebuffer());
		frameBuffer->RenderOffscreenTarget(CommandBuffer, vertexArray, imageIndex);

		vkDeviceWaitIdle(volkGetLoadedDevice());

		TransitionImageLayout(CommandBuffer, SwapchainRef.Info().Images[imageIndex],
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		vkCmdBeginRenderingKHR(CommandBuffer, &RenderingInfo);

		if (ImGui::GetDrawData() != nullptr)
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer);

		vkCmdEndRenderingKHR(CommandBuffer);

		TransitionImageLayout(CommandBuffer, SwapchainRef.Info().Images[imageIndex],
			VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		vkEndCommandBuffer(CommandBuffer);
	}

	void Frame::BuildRenderingInfo()
	{
		RenderingInfo.flags = VkRenderingFlagsKHR();
		RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		RenderingInfo.renderArea = VkRect2D({ 0,0 }, SwapchainRef.Info().Extent);
		RenderingInfo.layerCount = 1;
		RenderingInfo.viewMask = 0;
		RenderingInfo.colorAttachmentCount = 1;
		RenderingInfo.pColorAttachments = &ColorAttachment;
	}

	void Frame::BuildColorAttachment(uint32_t imageIndex)
	{
		ColorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		ColorAttachment.imageView = SwapchainRef.Info().ImageViews[imageIndex];
		ColorAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		ColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		ColorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		ColorAttachment.clearValue = { 0.1f, 0.1f, 0.1f, 1.0f };
	}

}