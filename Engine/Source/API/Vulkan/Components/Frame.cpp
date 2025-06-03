#include "kepch.h"

#include "Frame.h"
#include "Synchronization.h"
#include "Swapchain.h"

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

	void Frame::RecordCommandBuffer(uint32_t imageIndex)
	{
		vkResetCommandBuffer(CommandBuffer, NULL);
		BuildColorAttachment(imageIndex);
		BuildRenderingInfo();

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		vkBeginCommandBuffer(CommandBuffer, &beginInfo);

		TransitionImageLayout(CommandBuffer, SwapchainRef.Info().Images[imageIndex],
			VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VkAccessFlagBits::VK_ACCESS_NONE, VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		AnnoyingBoilerplateThatDynamicRenderingWasMeantToSpareUs();

		vkCmdBeginRenderingKHR(CommandBuffer, &RenderingInfo);

		VkShaderStageFlagBits stages[2] = {
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT
		};

		vkCmdBindShadersEXT(CommandBuffer, Shaders.size(), stages, Shaders.data());

		//vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

		vkCmdDraw(CommandBuffer, 3, 1, 0, 0);

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

	void Frame::AnnoyingBoilerplateThatDynamicRenderingWasMeantToSpareUs()
	{
		if (SwapchainRef.Info().Extent.width != 0 && SwapchainRef.Info().Extent.height != 0)
		{
			VkViewport viewport = VkViewport
			{
				.x = 0.0f, .y = 0.0f,
				.width = (float)SwapchainRef.Info().Extent.width,
				.height = (float)SwapchainRef.Info().Extent.height,
				.minDepth = 0.0f, .maxDepth = 1.0f
			};
			vkCmdSetViewportWithCountEXT(CommandBuffer, 1, &viewport);
		}

		VkRect2D scissor = VkRect2D({ 0,0 }, SwapchainRef.Info().Extent);
		vkCmdSetScissorWithCountEXT(CommandBuffer, 1, &scissor);

		VkVertexInputBindingDescription2EXT bindingDesc = {
			.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
			.binding = 0,
			.stride = sizeof(float) * 3, // 3 floats (x, y, z)
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
			.divisor = 1,
		};

		VkVertexInputAttributeDescription2EXT attributeDesc = {
			.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT, // vec3
			.offset = 0,
		};

		vkCmdSetVertexInputEXT(CommandBuffer, 1, &bindingDesc, 1, &attributeDesc);

		vkCmdSetRasterizerDiscardEnableEXT(CommandBuffer, VK_FALSE);
		vkCmdSetPolygonModeEXT(CommandBuffer, VK_POLYGON_MODE_FILL);
		vkCmdSetRasterizationSamplesEXT(CommandBuffer, VK_SAMPLE_COUNT_1_BIT);

		const VkSampleMask sampleMask = VkSampleMask();
		vkCmdSetSampleMaskEXT(CommandBuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);

		vkCmdSetAlphaToCoverageEnableEXT(CommandBuffer, VK_FALSE);
		vkCmdSetCullModeEXT(CommandBuffer, VK_CULL_MODE_NONE);
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
		equation.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		equation.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		equation.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Must be valid!
		equation.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
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