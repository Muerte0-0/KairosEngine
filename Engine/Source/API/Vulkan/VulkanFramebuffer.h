#pragma once
#include "Engine/Renderer/Framebuffer.h"

#include "volk.h"

#include "backends/imgui_impl_vulkan.h"

namespace Kairos
{
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec);
		virtual ~VulkanFramebuffer() {};

		void Invalidate();

		// Not Needed for Vulkan
		virtual void Bind() override {};
		virtual void Unbind() override {};

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override { return 0; }

		bool RenderOffscreenTarget(VkCommandBuffer commandBuffer, const Ref<class VertexArray>& vertexArray, uint32_t imageIndex);
		void CleanupFrameBuffer();

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override {}

		virtual uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const override;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		struct FrameResources
		{
			VkImage Image = VK_NULL_HANDLE;
			VkDeviceMemory Memory = VK_NULL_HANDLE;
			VkImageView ImageView = VK_NULL_HANDLE;
			VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
			bool DescriptorValid = false;
		};

		std::vector<FrameResources> m_FrameResources;
		std::deque<std::function<void(VkDevice)>> m_DeletionQueue;
	};
}