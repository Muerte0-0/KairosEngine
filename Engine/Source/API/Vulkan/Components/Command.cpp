#include "kepch.h"

#include "Command.h"
#include "Swapchain.h"
#include "Image.h"

#include "volk.h"

namespace Kairos
{
	void CreateCommandPool(VulkanContext* vctx)
	{
        VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT ,
        .queueFamilyIndex = vctx->FindQueueFamilies().graphics_family
        };

        VK_CHECK(vkCreateCommandPool(vctx->GetVkContext().device, &poolInfo, nullptr, &vctx->GetVkContext().commandPool));

		VkCommandPool handle = vctx->GetVkContext().commandPool;

		vctx->GetDeviceDeletionQueue().push_back([handle](VkDevice device)
			{
				vkDestroyCommandPool(device, handle, nullptr);
				KE_CORE_INFO("Deleted Command Pool!");
			});
	}

	void CreateCommandBuffers(VulkanContext* vctx)
	{
        vctx->GetVkContext().commandBuffers.resize(vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().images.size());

        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = vctx->GetVkContext().commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = (uint32_t)vctx->GetVkContext().commandBuffers.size()
        };
        VK_CHECK(vkAllocateCommandBuffers(vctx->GetVkContext().device, &allocInfo, vctx->GetVkContext().commandBuffers.data()));

        vctx->GetDeviceDeletionQueue().push_back([vctx](VkDevice device)
            {
                vkFreeCommandBuffers(device, vctx->GetVkContext().commandPool, (uint32_t)vctx->GetVkContext().commandBuffers.size(), vctx->GetVkContext().commandBuffers.data());
                KE_CORE_INFO("Deleted Command Buffers!");
			});
	}

    void CreateSyncObjects(VulkanContext* vctx)
    {
        vctx->GetVkContext().imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        vctx->GetVkContext().renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        vctx->GetVkContext().inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        vctx->GetVkContext().imagesInFlight.resize(vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().images.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT // Start signaled for first frame
        };

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {

            VK_CHECK(vkCreateSemaphore(vctx->GetVkContext().device, &semaphoreInfo, nullptr, &vctx->GetVkContext().imageAvailableSemaphores[i]));
            VK_CHECK(vkCreateSemaphore(vctx->GetVkContext().device, &semaphoreInfo, nullptr, &vctx->GetVkContext().renderFinishedSemaphores[i]));
            VK_CHECK(vkCreateFence(vctx->GetVkContext().device, &fenceInfo, nullptr, &vctx->GetVkContext().inFlightFences[i]));

            vctx->GetDeviceDeletionQueue().push_back([vctx, i](VkDevice device)
                {
                    vkDestroySemaphore(vctx->GetVkContext().device, vctx->GetVkContext().imageAvailableSemaphores[i], nullptr);
                    vkDestroySemaphore(vctx->GetVkContext().device, vctx->GetVkContext().renderFinishedSemaphores[i], nullptr);
                    vkDestroyFence(vctx->GetVkContext().device, vctx->GetVkContext().inFlightFences[i], nullptr);
                });
        }
    }

    VkCommandBuffer BeginSingleTimeCommands(VulkanContext* vctx)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = vctx->GetVkContext().commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(vctx->GetVkContext().device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void EndSingleTimeCommands(VulkanContext* vctx, VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(vctx->GetVkContext().graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(vctx->GetVkContext().graphicsQueue);

        vkFreeCommandBuffers(vctx->GetVkContext().device, vctx->GetVkContext().commandPool, 1, &commandBuffer);
    }

    void RenderFrame(VulkanContext* vctx)
    {
        vkWaitForFences(vctx->GetVkContext().device, 1, &vctx->GetVkContext().inFlightFences[vctx->GetVkContext().currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            vctx->GetVkContext().device,
            vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().swapchain,
            UINT64_MAX,
            vctx->GetVkContext().imageAvailableSemaphores[vctx->GetVkContext().currentFrame],
            VK_NULL_HANDLE,
            &imageIndex
        );

		// Handle swapchain recreation if Out of Date
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            vctx->GetVkContext().m_Swapchain.RecreateSwapchain(vctx);
            return;
        }

        vkResetFences(vctx->GetVkContext().device, 1, &vctx->GetVkContext().inFlightFences[vctx->GetVkContext().currentFrame]);
        VkCommandBuffer commandBuffer = vctx->GetVkContext().commandBuffers[imageIndex];
        vkResetCommandBuffer(commandBuffer, 0);
        RecordCommandBuffer(vctx, commandBuffer, imageIndex);

        VkPipelineStageFlags flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &vctx->GetVkContext().imageAvailableSemaphores[vctx->GetVkContext().currentFrame];
        submitInfo.pWaitDstStageMask = &flags;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &vctx->GetVkContext().renderFinishedSemaphores[vctx->GetVkContext().currentFrame];

        VK_CHECK(vkQueueSubmit(vctx->GetVkContext().graphicsQueue, 1, &submitInfo, vctx->GetVkContext().inFlightFences[vctx->GetVkContext().currentFrame]));

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &vctx->GetVkContext().renderFinishedSemaphores[vctx->GetVkContext().currentFrame],
            .swapchainCount = 1,
            .pSwapchains = &vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().swapchain,
            .pImageIndices = &imageIndex
        };

        result = vkQueuePresentKHR(vctx->GetVkContext().graphicsQueue, &presentInfo);

        vctx->GetVkContext().currentFrame = (vctx->GetVkContext().currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		// Handle swapchain recreation if Out of Date or Suboptimal
        if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR || result == VkResult::VK_SUBOPTIMAL_KHR)
        {
            vctx->GetVkContext().m_Swapchain.RecreateSwapchain(vctx);
            return;
        }
    }

    void RecordCommandBuffer(VulkanContext* vctx, VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        TransitionImageLayout(commandBuffer, vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().images[imageIndex],
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        VkRenderingAttachmentInfo colorAttachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().frames[imageIndex].ImageView,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {.color = { 0.1f, 0.1f, 0.1f, 1.0f } }
        };

        VkRenderingInfo renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {
                .offset = { 0, 0 },
                .extent = vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().extent
            },
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment,
        };

        vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);

        vkCmdEndRenderingKHR(commandBuffer);

		TransitionImageLayout(commandBuffer, vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().images[imageIndex],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));
    }
}