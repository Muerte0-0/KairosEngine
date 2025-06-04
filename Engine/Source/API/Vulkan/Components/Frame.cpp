#include "kepch.h"

#include "Frame.h"
#include "Synchronization.h"
#include "Swapchain.h"

#include "API/Vulkan/VulkanVertexArray.h"
#include "API/Vulkan/VulkanBuffer.h"

#include "Engine/Renderer/Renderer.h"

#include "volk.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

namespace Kairos
{
	Frame::Frame(Swapchain& swapchainRef, VkDevice logicalDevice, std::vector<VkShaderEXT>& shaders, VkCommandBuffer commandBuffer, std::deque<std::function<void(VkDevice)>>& deletionQueue)
		: SwapchainRef(swapchainRef), CommandBuffer(commandBuffer), Shaders(shaders)
	{
		ImageAquiredSemaphore = MakeSemaphore(logicalDevice, deletionQueue);
		RenderFinishedSemaphore = MakeSemaphore(logicalDevice, deletionQueue);
		RenderFinishedFence = MakeFence(logicalDevice, deletionQueue);
	}

	void Frame::RecordCommandBuffer(uint32_t imageIndex, const Ref<VertexArray>& vertexArray)
	{
		vkResetCommandBuffer(CommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		BuildColorAttachment(imageIndex);
		BuildRenderingInfo();

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		vkBeginCommandBuffer(CommandBuffer, &beginInfo);

		TransitionImageLayout(CommandBuffer, SwapchainRef.Info().Images[imageIndex],
			VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VkAccessFlagBits::VK_ACCESS_NONE, VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		vkCmdBeginRenderingKHR(CommandBuffer, &RenderingInfo);

		VkShaderStageFlagBits stages[2] = {
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT
		};

		vkCmdBindShadersEXT(CommandBuffer, Shaders.size(), stages, Shaders.data());

		for (const auto vertexBuffer : vertexArray->GetVertexBuffers())
		{
			VulkanVertexBuffer* vulkanVertexBuffer = static_cast<VulkanVertexBuffer*>(vertexBuffer.get());

			AnnoyingBoilerplateThatDynamicRenderingWasMeantToSpareUs(vulkanVertexBuffer->GetLayout());

			VkDeviceSize offsets = 0;//vulkanVertexBuffer->GetLayout().GetStride();

			vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &vulkanVertexBuffer->GetVertexBuffer(), &offsets);
		}

		VulkanIndexBuffer* vulkanIndexBuffer = static_cast<VulkanIndexBuffer*>(vertexArray->GetIndexBuffer().get());

		if (vulkanIndexBuffer != nullptr)
			vkCmdBindIndexBuffer(CommandBuffer, vulkanIndexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(CommandBuffer, 3, 1, 0, 0, 0);

		if (ImGui::GetDrawData() != nullptr)
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer);

		vkCmdEndRenderingKHR(CommandBuffer);

		TransitionImageLayout(CommandBuffer, SwapchainRef.Info().Images[imageIndex],
			VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

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

	void Frame::AnnoyingBoilerplateThatDynamicRenderingWasMeantToSpareUs(const BufferLayout& layout)
	{
		VkVertexInputBindingDescription2EXT bindingDescription = GetBindingDescription(layout);
		std::vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions = GetAttributeDescriptions(layout);

		vkCmdSetVertexInputEXT(CommandBuffer, 1, &bindingDescription, attributeDescriptions.size(), attributeDescriptions.data());

		if (SwapchainRef.Info().Extent.width != 0 && SwapchainRef.Info().Extent.height != 0)
		{
			VkViewport viewport = VkViewport
			{
				.x = 0.0f, .y = (float)SwapchainRef.Info().Extent.height,
				.width = (float)SwapchainRef.Info().Extent.width,
				.height = -(float)SwapchainRef.Info().Extent.height,
				.minDepth = 0.0f, .maxDepth = 1.0f
			};

			vkCmdSetViewportWithCountEXT(CommandBuffer, 1, &viewport);
		}

		VkRect2D scissor = VkRect2D({ 0,0 }, SwapchainRef.Info().Extent);
		vkCmdSetScissorWithCountEXT(CommandBuffer, 1, &scissor);

		vkCmdSetRasterizerDiscardEnableEXT(CommandBuffer, VK_FALSE);
		vkCmdSetPolygonModeEXT(CommandBuffer, VK_POLYGON_MODE_FILL);
		vkCmdSetRasterizationSamplesEXT(CommandBuffer, VK_SAMPLE_COUNT_1_BIT);

		uint32_t sampleMask = 1;
		vkCmdSetSampleMaskEXT(CommandBuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);

		vkCmdSetAlphaToCoverageEnableEXT(CommandBuffer, VK_FALSE);
		vkCmdSetCullModeEXT(CommandBuffer, VK_CULL_MODE_NONE);
		vkCmdSetFrontFaceEXT(CommandBuffer, VK_FRONT_FACE_CLOCKWISE);
		vkCmdSetDepthTestEnableEXT(CommandBuffer, VK_FALSE);
		vkCmdSetDepthWriteEnableEXT(CommandBuffer, VK_FALSE);
		vkCmdSetDepthBiasEnableEXT(CommandBuffer, VK_FALSE);
		vkCmdSetStencilTestEnableEXT(CommandBuffer, VK_FALSE);
		vkCmdSetPrimitiveTopologyEXT(CommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		vkCmdSetPrimitiveRestartEnableEXT(CommandBuffer, VK_FALSE);

		VkBool32 colorBlendEnable = VK_FALSE;
		vkCmdSetColorBlendEnableEXT(CommandBuffer, 0, 1, &colorBlendEnable);

		VkColorBlendEquationEXT equation;
		equation.colorBlendOp = VK_BLEND_OP_ADD;
		equation.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		equation.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		equation.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		equation.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		equation.alphaBlendOp = VK_BLEND_OP_ADD;

		vkCmdSetColorBlendEquationEXT(CommandBuffer, 0, 1, &equation);

		VkColorComponentFlags colorWriteMask = 
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		vkCmdSetColorWriteMaskEXT(CommandBuffer, 0, 1, &colorWriteMask);
	}

}