#pragma once
#include "Engine/Renderer/RHI/RenderAPI.h"

#include <vulkan/vulkan_raii.hpp>

#include "Components/VulkanDevice.h"
#include "Components/VulkanCommand.h"
#include "Components/VulkanSwapchain.h"

#include "VulkanFramebuffer.h"
#include "VulkanGraphicsPipeline.h"

#include "Engine/Renderer/RHI/Shader.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

namespace Engine
{	
	class VulkanRenderAPI : public RenderAPI
	{
	public:
		void Init(void* windowHandle, const std::filesystem::path& shaderDirectory) override;
		
		void BeginScene() override;
		void DrawFrame() override;
		void EndScene() override;
		
		void WindowResized() override;
		void ResizeFramebuffer(uint32_t width, uint32_t height) override;
		
		// ------------------------------------------------------------------
		// Accessors
		// ------------------------------------------------------------------
		const vk::raii::Instance& GetInstance() const { return m_Instance; }
		const vk::raii::SurfaceKHR& GetSurface() const { return m_Surface; }
		
		VulkanDevice* GetVulkanDevice() const { return m_VulkanDevice.get(); }
		VulkanCommand* GetVulkanCommand() const { return m_VulkanCommand.get(); }
		VulkanSwapchain* GetVulkanSwapchain() const { return m_VulkanSwapchain.get(); }
		
		Framebuffer*      GetFramebuffer()      override { return m_ViewportFramebuffer.get(); }
		GraphicsPipeline* GetGraphicsPipeline() override { return m_ViewportPipeline.get(); }
		ShaderLibrary*    GetShaderLibrary()    override { return &m_ShaderLibrary; }
		
		const std::vector<vk::raii::Buffer>& GetUniformBuffers() const { return m_UniformBuffers; }
		
		const vk::raii::CommandBuffer& GetActiveCommandBuffer() const { return m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex]; }
		
		API GetType() override { return API::Vulkan; }
		
	private:
		vk::raii::Context					m_Context;
		vk::raii::Instance					m_Instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT	m_DebugMessenger = nullptr;
		vk::raii::SurfaceKHR				m_Surface = nullptr;
		
		Scope<VulkanDevice>					m_VulkanDevice = nullptr;
		Scope<VulkanCommand>				m_VulkanCommand = nullptr;
		Scope<VulkanSwapchain>				m_VulkanSwapchain = nullptr;
		
		Scope<Framebuffer>					m_ViewportFramebuffer = nullptr;
		Scope<GraphicsPipeline>				m_ViewportPipeline    = nullptr;
		ShaderLibrary						m_ShaderLibrary;
		
		std::vector<vk::raii::Buffer>		m_UniformBuffers;
		std::vector<vk::raii::DeviceMemory>	m_UniformBuffersMemory;
		std::vector<void*>					m_UniformBuffersMapped;
		
		uint32_t							m_CurrentImageIndex = 0;
		uint32_t							m_CurrentFrameIndex = 0;
		
		bool								m_FrameValid = true;
		bool								m_FramebufferResized = false;
		bool								m_ViewportFramebufferResizePending = false;
		FramebufferSpecification			m_PendingViewportFramebufferSpecification{};
		
		std::filesystem::path				m_ShaderDirectory;
		
		void CreateInstance();
		static vector<const char*> GetRequiredInstanceExtensions();
		void SetupDebugMessenger();
		void CreateSurface(void* windowHandle);
		
		void ApplyPendingFramebufferResize();
		void CreateViewportFramebuffer();
		void CreateGraphicsPipeline();
		
		void BeginSwapchainRendering(vk::CommandBuffer commandBuffer) const;
		
		// Will Remove This Later -- Temp Stuff
		void CreateUniformBuffers();
		void UpdateUniformBuffer(uint32_t currentImage) const;
		
		void CreateSquareMesh();
		void DrawMesh(vk::CommandBuffer commandBuffer, const Ref<Mesh>& mesh) const;
		
		Ref<Mesh> m_CubeMesh;
	};
}
