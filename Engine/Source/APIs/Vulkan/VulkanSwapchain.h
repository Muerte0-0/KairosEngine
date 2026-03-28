#pragma once
#include <backends/imgui_impl_vulkan.h>

#include "VulkanDevice.h"

#include <GLFW/glfw3.h>

namespace Engine
{
	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VulkanDevice &device, GLFWwindow* windowHandle);
		~VulkanSwapchain();

		bool Create();
		bool CreateImageViews();
		void Cleanup();
		bool Recreate();
		void CreateColorResources();
		void CreateDepthResources();
		bool SetupDynamicRendering();
		
		vk::raii::SwapchainKHR &GetSwapChain() { return m_SwapChain; }
		const std::vector<vk::Image> &GetSwapChainImages() const { return m_SwapChainImages; }
		const vk::Format& GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
		const vk::Extent2D& GetSwapChainExtent() const { return m_SwapChainExtent; }
		const std::vector<vk::raii::ImageView> &GetSwapChainImageViews() const { return m_SwapChainImageViews; }
		const vk::raii::Image &GetColorImage() const { return m_ColorImage; }
		const vk::raii::ImageView &GetColorImageView() const { return m_ColorImageView; }
		const vk::raii::Image &GetDepthImage() const { return m_DepthImage; }
		const vk::raii::ImageView &GetDepthImageView() const { return m_DepthImageView; }
		const vk::Format& GetDepthImageFormat() const { return m_DepthImageFormat; }
		const vk::SampleCountFlagBits &GetMSAASamples() const { return m_MSAA_Samples; }
		const vk::RenderingInfo &GetRenderingInfo(uint32_t imageIndex);
		
	private:
		VulkanDevice &m_VulkanDevice;
		GLFWwindow* m_Window;

		vk::raii::SwapchainKHR m_SwapChain = nullptr;
		std::vector<vk::Image> m_SwapChainImages;
		vk::Format m_SwapChainImageFormat = vk::Format::eUndefined;
		vk::Extent2D m_SwapChainExtent = {0, 0};
		std::vector<vk::raii::ImageView> m_SwapChainImageViews;
		std::vector<vk::ImageLayout> m_SwapChainImageLayouts;
		
		vk::SampleCountFlagBits m_MSAA_Samples = vk::SampleCountFlagBits::e1;
		
		vk::raii::Image m_ColorImage = nullptr;
		vk::raii::DeviceMemory m_ColorImageMemory = nullptr;
		vk::raii::ImageView m_ColorImageView = nullptr;

		vk::raii::Image m_DepthImage = nullptr;
		vk::raii::DeviceMemory m_DepthImageMemory = nullptr;
		vk::raii::ImageView m_DepthImageView = nullptr;
		vk::Format m_DepthImageFormat = vk::Format::eUndefined;
		
		vk::RenderingInfo m_RenderingInfo;
		vector<vk::RenderingAttachmentInfo> m_ColorAttachments;
		vk::RenderingAttachmentInfo m_DepthAttachment;
		
		vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const vector<vk::SurfaceFormatKHR> &availableFormats) const;
		vk::PresentModeKHR ChooseSwapPresentMode(const vector<vk::PresentModeKHR> &availablePresentModes) const;
		vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const;
		
		vk::Format FindDepthFormat() const;
	};
}
