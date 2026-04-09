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

		vk::raii::SwapchainKHR &GetSwapChain() { return m_SwapChain; }
		const std::vector<vk::Image> &GetSwapChainImages() const { return m_SwapChainImages; }
		const vk::Format& GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
		const vk::Extent2D& GetSwapChainExtent() const { return m_SwapChainExtent; }
		const std::vector<vk::raii::ImageView> &GetSwapChainImageViews() const { return m_SwapChainImageViews; }

	private:
		VulkanDevice &m_VulkanDevice;
		GLFWwindow* m_Window;

		vk::raii::SwapchainKHR m_SwapChain = nullptr;
		std::vector<vk::Image> m_SwapChainImages;
		vk::Format m_SwapChainImageFormat = vk::Format::eUndefined;
		vk::Extent2D m_SwapChainExtent = {0, 0};
		std::vector<vk::raii::ImageView> m_SwapChainImageViews;
		std::vector<vk::ImageLayout> m_SwapChainImageLayouts;

		vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const vector<vk::SurfaceFormatKHR> &availableFormats) const;
		vk::PresentModeKHR ChooseSwapPresentMode(const vector<vk::PresentModeKHR> &availablePresentModes) const;
		vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const;
	};
}
