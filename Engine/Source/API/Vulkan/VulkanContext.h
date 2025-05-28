#pragma once
#include "Engine/Renderer/GraphicsContext.h"

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Kairos
{
	struct QueueFamilyIndices
	{
		uint32_t graphics_family;
		uint32_t present_family;
		uint32_t compute_family;
		bool has_compute;
	};

	struct VkContext 
	{
		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;

		VmaAllocator allocator;
		VkAllocationCallbacks* callbacks;

		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkQueue computeQueue;
	};

#define VK_CHECK(result) \
    if (result != VK_SUCCESS) { \
        KE_CORE_ERROR("Vulkan error: {}", (char*)result); \
        abort(); \
    }

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SetVSync(bool enabled) override;
		virtual void SwapBuffers() override;
		virtual void Cleanup() override;

		std::deque<std::function<void(VkInstance)>>& GetInstanceDeletionQueue() { return m_InstanceDeletionQueue; }
		std::deque<std::function<void(VkDevice)>>& GetDeviceDeletionQueue() { return m_DeviceDeletionQueue; }

		VkContext& GetVkContext() { return m_Context; }
	private:
		GLFWwindow* m_WindowHandle;

		VkContext m_Context;

		void Init_Instance();

		void Init_Device();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();

		QueueFamilyIndices FindQueueFamilies();

		void Init_Allocator();

		std::deque<std::function<void(VkInstance)>> m_InstanceDeletionQueue;
		std::deque<std::function<void(VkDevice)>> m_DeviceDeletionQueue;
	};
}