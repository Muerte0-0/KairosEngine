#pragma once
#include "Engine/Renderer/RHI/RenderAPI.h"

#include <vulkan/vulkan_raii.hpp>

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanCommand.h"

namespace Engine
{
	static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		vk::DebugUtilsMessageTypeFlagsEXT type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

		return vk::False;
	}
	
	class VulkanRenderAPI : public RenderAPI
	{
	public:
		void Init(void* windowHandle) override;
		void BeginFrame() override;
		void DrawFrame() override;
		void EndFrame() override;
		void WindowResized() override;
		
		/**
		 * @brief Get the Instance.
		 * @return The Instance.
		*/
		const vk::raii::Instance& GetInstance() const { return m_Instance; }
		
		
		/**
		 * @brief Get the Surface.
		 * @return The Surface KHR.
		*/
		const vk::raii::SurfaceKHR& GetSurface() const { return m_Surface; }
		
		/**
		 * @brief Get the Surface.
		 * @return The Surface KHR.
		*/
		VulkanDevice* GetVulkanDevice() { return m_VulkanDevice.get(); }
		
		/**
		 * @brief Get the Surface.
		 * @return The Surface KHR.
		*/
		VulkanSwapchain* GetVulkanSwapchain() { return m_VulkanSwapchain.get(); }
		
		/**
		 * @brief Get the Surface.
		 * @return The Surface KHR.
		*/
		VulkanCommand* GetVulkanCommand() { return m_VulkanCommand.get(); }
		
		const vk::raii::CommandBuffer& GetActiveCommandBuffer() const {return m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex]; }
		
	private:
		vk::raii::Context m_Context;
		vk::raii::Instance m_Instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
		vk::raii::SurfaceKHR m_Surface = nullptr;
		
		Scope<VulkanDevice> m_VulkanDevice = nullptr;
		Scope<VulkanSwapchain> m_VulkanSwapchain = nullptr;
		Scope<VulkanCommand> m_VulkanCommand = nullptr;
		
		uint32_t m_CurrentImageIndex = 0;
		uint32_t m_CurrentFrameIndex = 0;
		
		bool m_FrameValid = true;
		bool m_FramebufferResized = false;
		
		void CreateInstance();
		vector<const char*> GetRequiredInstanceExtensions() const;
		void SetupDebugMessenger();
		void CreateSurface(void* windowHandle);
	};
}
