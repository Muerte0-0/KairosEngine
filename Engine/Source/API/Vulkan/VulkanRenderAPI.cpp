#include "kepch.h"
#include "VulkanRenderAPI.h"

#include "Engine/Core/Application.h"
#include "VulkanContext.h"
#include "Components/Command.h"

#include "volk.h"

namespace Kairos
{

	void VulkanRenderAPI::Init()
	{
		
	}

	void VulkanRenderAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		vctx->GetVkContext().Swapchain.RecreateSwapchain(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().PhysicalDevice, vctx->GetVkContext().Surface, vctx->GetWindowHandle());
	}

	void VulkanRenderAPI::Clear()
	{
		
	}

	void VulkanRenderAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		if (Application::Get().IsMinimized())
			return;

		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		if (vctx->GetVkContext().Swapchain.Outdated)
			vctx->GetVkContext().Swapchain.RecreateSwapchain(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().PhysicalDevice, vctx->GetVkContext().Surface, vctx->GetWindowHandle());

		vkWaitForFences(vctx->GetVkContext().LogicalDevice, 1, &vctx->GetVkContext().RenderFinishedFence, VK_TRUE, UINT64_MAX);

		vkResetFences(vctx->GetVkContext().LogicalDevice, 1, &vctx->GetVkContext().RenderFinishedFence);

		uint32_t imageIndex;
		VkResult aquireResult = vkAcquireNextImageKHR(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().Swapchain.Info().Swapchain, UINT64_MAX, vctx->GetVkContext().ImageAquiredSemaphore, VK_NULL_HANDLE, &imageIndex);
		CurrentFrameIndex = imageIndex;

		if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			vctx->GetVkContext().Swapchain.Outdated = true;
			return;
		}

		vctx->GetVkContext().Frames[imageIndex].RecordCommandBuffer(imageIndex, vertexArray);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vctx->GetVkContext().Frames[imageIndex].CommandBuffer;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &vctx->GetVkContext().ImageAquiredSemaphore;
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &vctx->GetVkContext().Frames[imageIndex].RenderFinishedSemaphore;

		vkQueueSubmit(vctx->GetVkContext().GraphicsQueue, 1, &submitInfo, vctx->GetVkContext().RenderFinishedFence);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &vctx->GetVkContext().Swapchain.Info().Swapchain;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &vctx->GetVkContext().Frames[imageIndex].RenderFinishedSemaphore;

		vkWaitForFences(vctx->GetVkContext().LogicalDevice, 1, &vctx->GetVkContext().RenderFinishedFence, VK_TRUE, UINT64_MAX);
		VkResult presentResult = vkQueuePresentKHR(vctx->GetVkContext().GraphicsQueue, &presentInfo);

		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
		{
			vctx->GetVkContext().Swapchain.Outdated = true;
			return;
		}
	}
}