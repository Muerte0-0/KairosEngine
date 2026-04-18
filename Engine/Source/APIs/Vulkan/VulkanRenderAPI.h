#pragma once
#include "Engine/Renderer/RHI/RenderAPI.h"

#include <vulkan/vulkan_raii.hpp>

#include "Components/VulkanDevice.h"
#include "Components/VulkanCommand.h"
#include "Components/VulkanSwapchain.h"

#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/RHI/Shader.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

namespace Engine
{
	class VulkanFramebuffer;

	class VulkanRenderAPI : public RenderAPI
	{
	public:
		void Init(void* windowHandle, const std::filesystem::path& shaderDirectory) override;

		void BeginScene() override;
		void BeginPass(const RenderPass& renderPass) override;
		void EndPass() override;
		void DrawMesh(
			const Framebuffer& framebuffer,
			const GraphicsPipeline& pipeline,
			const Mesh& mesh,
			const glm::mat4& modelTransform,
			const UniformBufferObject& uniformBufferObject,
			const std::vector<Ref<Material>>& materials = {}) override;
		void DrawFrame()  override;
		void EndScene()   override;

		void WindowResized() override;

		ShaderLibrary* GetShaderLibrary() override { return &m_ShaderLibrary; }
		const std::filesystem::path& GetShaderDirectory() const override { return m_ShaderDirectory; }
		TextureFormat GetDefaultColorFormat() const override;
		TextureFormat GetDefaultDepthFormat() const override;
		void WaitIdle() override;
		void ReleaseStaticResources() override;
		API GetType() override { return API::Vulkan; }

		// Material factory + shared descriptor set layout for set 1
		Ref<Material> CreateMaterial() override;

		// Shared set-1 DSL — created once in Init(), referenced by every VulkanMaterial
		// and by VulkanGraphicsPipeline when building the pipeline layout.
		const vk::raii::DescriptorSetLayout& GetMaterialDescriptorSetLayout() const
		{
			return m_MaterialDescriptorSetLayout;
		}

		// ------------------------------------------------------------------
		// Accessors used by Vulkan-side helpers (Scene Renderer, Framebuffer)
		// ------------------------------------------------------------------
		const vk::raii::Instance&    GetInstance() const { return m_Instance; }
		const vk::raii::SurfaceKHR&  GetSurface()  const { return m_Surface;  }

		VulkanDevice*    GetVulkanDevice()    const { return m_VulkanDevice.get();    }
		VulkanCommand*   GetVulkanCommand()   const { return m_VulkanCommand.get();   }
		VulkanSwapchain* GetVulkanSwapchain() const { return m_VulkanSwapchain.get(); }

		const vk::raii::CommandBuffer& GetActiveCommandBuffer() const
		{
			return m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex];
		}

		uint32_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }

		const std::vector<vk::raii::Buffer>& GetUniformBuffers() const { return m_UniformBuffers; }
		const std::vector<void*>& GetMappedUniformBuffers() const { return m_UniformBuffersMapped; }

	private:
		vk::raii::Context                m_Context;
		vk::raii::Instance               m_Instance        = nullptr;
		vk::raii::DebugUtilsMessengerEXT m_DebugMessenger  = nullptr;
		vk::raii::SurfaceKHR             m_Surface         = nullptr;

		Scope<VulkanDevice>    m_VulkanDevice    = nullptr;
		Scope<VulkanCommand>   m_VulkanCommand   = nullptr;
		Scope<VulkanSwapchain> m_VulkanSwapchain = nullptr;

		ShaderLibrary m_ShaderLibrary;

		vk::raii::DescriptorSetLayout m_MaterialDescriptorSetLayout = nullptr;

		std::vector<vk::raii::Buffer>       m_UniformBuffers;
		std::vector<vk::raii::DeviceMemory> m_UniformBuffersMemory;
		std::vector<void*>                  m_UniformBuffersMapped;

		uint32_t m_CurrentImageIndex = 0;
		uint32_t m_CurrentFrameIndex = 0;
		bool     m_FrameValid        = true;
		bool     m_SwapchainDirty    = false;
		bool     m_OffscreenPassActive = false;
		VulkanFramebuffer* m_ActiveFramebuffer = nullptr;

		std::filesystem::path m_ShaderDirectory;

		void CreateInstance();
		static std::vector<const char*> GetRequiredInstanceExtensions();
		void SetupDebugMessenger();
		void CreateSurface(void* windowHandle);
		void CreateUniformBuffers();
		void CreateMaterialDescriptorSetLayout();

		void BeginSwapchainRendering(vk::CommandBuffer commandBuffer) const;
	};
}
