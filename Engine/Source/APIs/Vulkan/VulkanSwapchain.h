#pragma once
#include "VulkanDevice.h"

#include <GLFW/glfw3.h>

namespace Engine
{
	class VulkanSwapchain
	{
	public:
		/**
		  * @param device The Vulkan device.
		*/
		VulkanSwapchain(VulkanDevice &device, GLFWwindow* windowHandle);

		/**
		 * @brief Destructor.
		*/
		~VulkanSwapchain();

		/**
		 * @brief Create the swap chain.
		 * @return True if the swap chain was created successfully, false otherwise.
		*/
		bool Create();

		/**
		 * @brief Create image views for the swap chain images.
		 * @return True if the image views were created successfully, false otherwise.
		*/
		bool CreateImageViews();

		/**
		 * @brief Clean up the swap chain.
		*/
		void Cleanup();

		/**
		 * @brief Recreate the swap chain.
		 * @return True, if the swap chain was recreated successfully, false otherwise.
		*/
		bool Recreate();

		/**
		 * @brief Create the Color Resources
		*/
		void CreateColorResources();
		
		/**
		 * @brief Create the Depth Resources
		*/
		void CreateDepthResources();
		
		/**
		 * @brief Get the swap chain.
		 * @return The swap chain.
		*/
		vk::raii::SwapchainKHR &GetSwapChain() { return m_SwapChain; }

		/**
		 * @brief Get the swap chain images.
		 * @return The swap chain images.
		*/
		const std::vector<vk::Image> &GetSwapChainImages() const { return m_SwapChainImages; }

		/**
		 * @brief Get the swap chain image format.
		 * @return The swap chain image format.
		*/
		vk::Format GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }

		/**
		 * @brief Get the swap chain extent.
		 * @return The swap chain extent.
		*/
		vk::Extent2D GetSwapChainExtent() const { return m_SwapChainExtent; }

		/**
		 * @brief Get the swap chain image views.
		 * @return The swap chain image views.
		*/
		const std::vector<vk::raii::ImageView> &GetSwapChainImageViews() const { return m_SwapChainImageViews; }
		
	private:
		// Vulkan device
		VulkanDevice &m_VulkanDevice;
		GLFWwindow* m_Window;

		// Swap chain
		vk::raii::SwapchainKHR m_SwapChain = nullptr;
		std::vector<vk::Image> m_SwapChainImages;
		vk::Format m_SwapChainImageFormat = vk::Format::eUndefined;
		vk::Extent2D m_SwapChainExtent      = {0, 0};
		std::vector<vk::raii::ImageView> m_SwapChainImageViews;
		// Tracked layouts for swapchain images (VVL requires correct oldLayout in barriers).
		// Initialized at swapchain creation and updated as we transition.
		std::vector<vk::ImageLayout> m_SwapChainImageLayouts;
		
		vk::raii::Image m_ColorImage = nullptr;
		vk::raii::DeviceMemory m_ColorImageMemory = nullptr;
		vk::raii::ImageView m_ColorImageView = nullptr;

		vk::raii::Image m_DepthImage = nullptr;
		vk::raii::DeviceMemory m_DepthImageMemory = nullptr;
		vk::raii::ImageView m_DepthImageView = nullptr;
		
		// Helper functions
		vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const vector<vk::SurfaceFormatKHR> &availableFormats) const;
		vk::PresentModeKHR ChooseSwapPresentMode(const vector<vk::PresentModeKHR> &availablePresentModes) const;
		vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const;
		
		vk::Format FindDepthFormat() const;
	};
}
