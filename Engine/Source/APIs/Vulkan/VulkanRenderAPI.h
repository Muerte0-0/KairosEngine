#pragma once
#include "Engine/Renderer/RHI/RenderAPI.h"

#include <vulkan/vulkan_raii.hpp>

#include "Components/VulkanDevice.h"
#include "Components/VulkanCommand.h"
#include "Components/VulkanSwapchain.h"

#include "VulkanFramebuffer.h"
#include "VulkanGraphicsPipeline.h"

namespace Engine
{	
	class VulkanRenderAPI : public RenderAPI
	{
	public:
		void Init(void* windowHandle, const std::filesystem::path& shaderDirectory) override;
		
		void BeginFrame() override;
		void DrawFrame() override;
		void EndFrame() override;
		
		void WindowResized() override;
		void ResizeFramebuffer(uint32_t width, uint32_t height) override;
		
		const vk::raii::Instance& GetInstance() const { return m_Instance; }
		const vk::raii::SurfaceKHR& GetSurface() const { return m_Surface; }
		
		VulkanDevice* GetVulkanDevice() const { return m_VulkanDevice.get(); }
		VulkanCommand* GetVulkanCommand() const { return m_VulkanCommand.get(); }
		VulkanSwapchain* GetVulkanSwapchain() const { return m_VulkanSwapchain.get(); }
		
		Framebuffer* GetFramebuffer() override { return m_ViewportFramebuffer.get(); }
		GraphicsPipeline* GetGraphicsPipeline() override { return m_ViewportPipeline.get(); }
		
		const vk::raii::CommandBuffer& GetActiveCommandBuffer() const { return m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex]; }
		
	private:
		vk::raii::Context m_Context;
		vk::raii::Instance m_Instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
		vk::raii::SurfaceKHR m_Surface = nullptr;
		
		Scope<VulkanDevice> m_VulkanDevice = nullptr;
		Scope<VulkanCommand> m_VulkanCommand = nullptr;
		Scope<VulkanSwapchain> m_VulkanSwapchain = nullptr;
		
		Scope<VulkanFramebuffer> m_ViewportFramebuffer = nullptr;
		Scope<VulkanGraphicsPipeline> m_ViewportPipeline = nullptr;
		
		uint32_t m_CurrentImageIndex = 0;
		uint32_t m_CurrentFrameIndex = 0;
		
		bool m_FrameValid = true;
		bool m_FramebufferResized = false;
		bool m_ViewportFramebufferResizePending = false;
		FramebufferSpecification m_PendingViewportFramebufferSpecification{};
		
		std::filesystem::path m_ShaderDirectory;
		
		void CreateInstance();
		vector<const char*> GetRequiredInstanceExtensions() const;
		void SetupDebugMessenger();
		void CreateSurface(void* windowHandle);
		
		void ApplyPendingFramebufferResize();
		void CreateViewportFramebuffer();
		void CreateGraphicsPipeline();
		
		void BeginSwapchainRendering(vk::CommandBuffer commandBuffer) const;
	};
}
