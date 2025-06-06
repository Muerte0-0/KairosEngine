#pragma once
#include "Engine/Renderer/Framebuffer.h"

#include <vulkan/vulkan.h>

#include "backends/imgui_impl_vulkan.h"

namespace Kairos
{
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec);
		virtual ~VulkanFramebuffer();

		void Invalidate();

		// Not Needed for Vulkan
		virtual void Bind() override {};
		virtual void Unbind() override {};

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override { return 0; }

		void Destroy();
		void DestroyOffscreenTarget();
		void RenderOffscreenTarget(VkCommandBuffer commandBuffer, const Ref<class VertexArray>& vertexArray);

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		std::vector<VkImage> m_ViewportImages;
		std::vector<VkDeviceMemory> m_DstImageMemory;
		std::vector<VkImageView> m_ViewportImageViews;

		std::vector<VkDescriptorSet> m_ImGuiDescriptorSets;
		VkDescriptorSetLayout m_DescriptorSetLayout;
	};
}