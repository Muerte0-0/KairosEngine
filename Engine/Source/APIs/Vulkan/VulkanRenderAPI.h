#pragma once
#include "Engine/Renderer/RHI/RenderAPI.h"

#include <vulkan/vulkan_raii.hpp>

#include "VulkanDevice.h"

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
		void EndFrame() override;
		void SetClearColor(float r, float g, float b, float a) override;
		void Clear() override;
		
	private:
		vk::raii::Context m_Context;
		vk::raii::Instance m_Instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
		vk::raii::SurfaceKHR m_Surface = nullptr;
		
		Scope<VulkanDevice> m_VulkanDevice = nullptr;
		
		void CreateInstance();
		std::vector<const char*> GetRequiredInstanceExtensions() const;
		void SetupDebugMessenger();
		void CreateSurface(void* windowHandle);
	};
}
