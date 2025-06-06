#include "kepch.h"
#include "VulkanFramebuffer.h"

#include "VulkanUtils.h"
#include "API/Vulkan/VulkanVertexArray.h"
#include "API/Vulkan/VulkanBuffer.h"
#include "API/Vulkan/VulkanShader.h"

#include "VulkanContext.h"
#include "Engine/Core/Application.h"
#include "Engine/Renderer/Renderer.h"

#include "Components/Image.h"
#include "Components/Command.h"
#include "Components/Descriptors.h"

#include "volk.h"

namespace Kairos
{
	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils
	{
		static VkImageType TextureTarget(bool multisampled)
		{
			return multisampled ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
		{
			
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:  return true;
			}

			return false;
		}

		static VkFormat KairosFBTextureFormatToGL(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::RGBA8:       return VK_FORMAT_R8G8B8A8_UINT;
			case FramebufferTextureFormat::RED_INTEGER: return VK_FORMAT_R8_UINT;
			}

			KE_CORE_ASSERT(false);
			return VK_FORMAT_MAX_ENUM;
		}

	}

	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec) : m_Specification(spec)
	{
		for (auto format : m_Specification.Attachments.Attachments)
		{
			if (Utils::IsDepthFormat(format.TextureFormat))
				m_DepthAttachmentSpecification = format;
			else
				m_ColorAttachmentSpecifications.emplace_back(format);
		}

		Invalidate();

		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		DescriptorSetLayoutBuilder descriptorSetLyoutBuilder(vctx->GetVkContext().LogicalDevice);
		descriptorSetLyoutBuilder.AddEntry(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		m_DescriptorSetLayout = descriptorSetLyoutBuilder.Build(vctx->GetDeviceDeletionQueue());
		
		PipelineLayoutBuilder pipelineLayoutBuilder(vctx->GetVkContext().LogicalDevice);
		pipelineLayoutBuilder.Add(m_DescriptorSetLayout);
		m_PipelineLayout = pipelineLayoutBuilder.Build(vctx->GetDeviceDeletionQueue());
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
	}

	void VulkanFramebuffer::Invalidate()
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		m_ViewportImages.resize(vctx->GetVkContext().Swapchain.Info().Images.size());
		m_DstImageMemory.resize(vctx->GetVkContext().Swapchain.Info().Images.size());

		for (uint32_t i = 0; i < vctx->GetVkContext().Swapchain.Info().Images.size(); i++)
		{
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
			imageInfo.extent.width = m_Specification.Width;
			imageInfo.extent.height = m_Specification.Height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VK_CHECK(vkCreateImage(vctx->GetVkContext().LogicalDevice, &imageInfo, nullptr, &m_ViewportImages[i]));

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(vctx->GetVkContext().LogicalDevice, m_ViewportImages[i], &memRequirements);

			VkMemoryAllocateInfo memAllocInfo{};
			memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memAllocInfo.allocationSize = memRequirements.size;
			memAllocInfo.memoryTypeIndex = FindMemoryType(vctx->GetVkContext().PhysicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VK_CHECK(vkAllocateMemory(vctx->GetVkContext().LogicalDevice, &memAllocInfo, nullptr, &m_DstImageMemory[i]));
			VK_CHECK(vkBindImageMemory(vctx->GetVkContext().LogicalDevice, m_ViewportImages[i], m_DstImageMemory[i], 0));

			VkCommandBuffer commandBuffer = BeginSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool);
			
			TransitionImageLayout(commandBuffer, m_ViewportImages[i],
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
			
			EndSingleTimeCommands(vctx->GetVkContext().LogicalDevice, commandBuffer, vctx->GetVkContext().CommandPool, vctx->GetVkContext().GraphicsQueue);
		}

		m_ViewportImageViews.resize(m_ViewportImages.size());
		m_ImGuiDescriptorSets.resize(m_ViewportImages.size());

		for (uint32_t i = 0; i < m_ViewportImages.size(); i++)
		{
			VkImageViewCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.image = m_ViewportImages[i];
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = VK_FORMAT_R8G8B8A8_UNORM;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.levelCount = 1;
			info.subresourceRange.layerCount = 1;
			VK_CHECK(vkCreateImageView(vctx->GetVkContext().LogicalDevice, &info, nullptr, &m_ViewportImageViews[i]));

			m_ImGuiDescriptorSets[i] = ImGui_ImplVulkan_AddTexture(vctx->GetVkContext().Sampler, m_ViewportImageViews[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			KE_CORE_WARN("Attempted to rezize framebuffer to {0}, {1}", width, height);
			return;
		}

		vkDeviceWaitIdle(volkGetLoadedDevice());

		m_Specification.Width = width;
		m_Specification.Height = height;

		DestroyOffscreenTarget();

		Invalidate();
	}

	void VulkanFramebuffer::DestroyOffscreenTarget()
	{
		VkDevice logicalDevice = volkGetLoadedDevice();

		for (uint32_t i = 0; i < m_ViewportImages.size(); i++)
		{
			if (m_ViewportImages[i] != VK_NULL_HANDLE)
			{
				vkDestroyImageView(logicalDevice, m_ViewportImageViews[i], nullptr);
				vkDestroyImage(logicalDevice, m_ViewportImages[i], nullptr);
				vkFreeMemory(logicalDevice, m_DstImageMemory[i], nullptr);
			}
		}

		for (auto descriptor : m_ImGuiDescriptorSets)
			ImGui_ImplVulkan_RemoveTexture(descriptor);

		m_ViewportImages.clear();
		m_ViewportImageViews.clear();
		m_DstImageMemory.clear();
	}

	void VulkanFramebuffer::RenderOffscreenTarget(VkCommandBuffer commandBuffer, const Ref<class VertexArray>& vertexArray)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		TransitionImageLayout(commandBuffer, m_ViewportImages[CurrentFrameIndex],
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		VkRenderingAttachmentInfo colorAttachment = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		colorAttachment.imageView = m_ViewportImageViews[CurrentFrameIndex];
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue.color = { 0.1f, 0.1f, 0.1f, 1.0f };

		VkRenderingInfo offscreenRenderingInfo = { VK_STRUCTURE_TYPE_RENDERING_INFO };
		offscreenRenderingInfo.renderArea = { {0, 0}, {m_Specification.Width, m_Specification.Height} };
		offscreenRenderingInfo.layerCount = 1;
		offscreenRenderingInfo.colorAttachmentCount = 1;
		offscreenRenderingInfo.pColorAttachments = &colorAttachment;

		vkCmdBeginRenderingKHR(commandBuffer, &offscreenRenderingInfo);

		VkShaderStageFlagBits stages[2] =
		{
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		auto shaders = Renderer::GetShaderLibrary().GetShaders();
		
		for (auto shader : shaders)
		{
			VulkanShader* vkShader = static_cast<VulkanShader*>(shader.second.get());

			vkCmdBindShadersEXT(commandBuffer, vkShader->GetShaders().size(), stages, vkShader->GetShaders().data());

			for (const auto vertexBuffer : vertexArray->GetVertexBuffers())
			{
				VulkanVertexBuffer* vulkanVertexBuffer = static_cast<VulkanVertexBuffer*>(vertexBuffer.get());

				VkExtent2D extent = {m_Specification.Width, m_Specification.Height};

				AnnoyingBoilerplateThatDynamicRenderingWasMeantToSpareUs(commandBuffer, vulkanVertexBuffer->GetLayout(), extent);

				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vulkanVertexBuffer->GetVertexBuffer(), &vulkanVertexBuffer->GetOffsets());
			}

			Ref<VulkanIndexBuffer> vulkanIndexBuffer = std::dynamic_pointer_cast<VulkanIndexBuffer>(vertexArray->GetIndexBuffer());

			if (vulkanIndexBuffer != nullptr)
				vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer, 3, 1, 0, 0, 0);
		}

		vkCmdEndRenderingKHR(commandBuffer);

		TransitionImageLayout(commandBuffer, m_ViewportImages[CurrentFrameIndex],
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}

	void VulkanFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		DestroyOffscreenTarget();

		Invalidate();
	}
	uint32_t VulkanFramebuffer::GetColorAttachmentRendererID(uint32_t index) const
	{
		return (ImTextureID)m_ImGuiDescriptorSets[CurrentFrameIndex];
	}
}