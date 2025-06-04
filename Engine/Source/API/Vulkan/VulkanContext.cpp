#include "kepch.h"
#include "VulkanContext.h"

#define VOLK_IMPLEMENTATION
#include "volk.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

//Components
#include "Components/Device.h"
#include "Components/Command.h"
#include "Components/Synchronization.h"
#include "Components/Image.h"

#include "Engine/Core/Application.h"

#include "backends/imgui_impl_vulkan.h"

namespace Kairos
{
	VulkanContext::VulkanContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle)
	{
		KE_CORE_ASSERT(windowHandle, "Window Handle is Null!");
	}
	
	void VulkanContext::Init()
	{
		if (volkInitialize() != VK_SUCCESS)
			KE_CORE_ERROR("Failed to initialize Vulkan");

		m_Context.Instance = CreateInstance(glfwGetWindowTitle(m_WindowHandle), m_InstanceDeletionQueue);
		volkLoadInstance(m_Context.Instance);
		m_Context.Surface = CreateSurface(m_Context.Instance, m_WindowHandle, m_InstanceDeletionQueue);

		m_Context.PhysicalDevice = ChoosePhysicalDevice(m_Context.Instance);
		m_Context.LogicalDevice = CreateLogicalDevice(m_Context.PhysicalDevice, m_Context.Surface, m_DeviceDeletionQueue);
		volkLoadDevice(m_Context.LogicalDevice);

		// Get queues
		QueueFamilyIndices indices = FindQueueFamilies(m_Context.PhysicalDevice, m_Context.Surface);
		vkGetDeviceQueue(m_Context.LogicalDevice, indices.GraphicsIndex, 0, &m_Context.GraphicsQueue);
		vkGetDeviceQueue(m_Context.LogicalDevice, indices.PresentIndex, 0, &m_Context.PresentQueue);
		if (indices.HasCompute)
			vkGetDeviceQueue(m_Context.LogicalDevice, indices.ComputeIndex, 0, &m_Context.ComputeQueue);

		Init_Allocator();

		int width, height;
		glfwGetWindowSize(m_WindowHandle, &width, &height);

		m_Context.Swapchain.CreateSwapchain(m_Context.LogicalDevice, m_Context.PhysicalDevice, m_Context.Surface, width, height);
		m_Context.Swapchain.CreateDescriptorPool(m_Context.LogicalDevice, m_DeviceDeletionQueue);

		m_Context.CommandPool = CreateCommandPool(m_Context.LogicalDevice, FindQueueFamilyIndex(m_Context.PhysicalDevice, m_Context.Surface, VK_QUEUE_GRAPHICS_BIT), m_DeviceDeletionQueue);

		m_Context.ImageAquiredSemaphore = MakeSemaphore(m_Context.LogicalDevice, m_DeviceDeletionQueue);
		m_Context.RenderFinishedSemaphore = MakeSemaphore(m_Context.LogicalDevice, m_DeviceDeletionQueue);
		m_Context.RenderFinishedFence = MakeFence(m_Context.LogicalDevice, m_DeviceDeletionQueue);

		// Logging Vulkan Info
		uint32_t version;
		vkEnumerateInstanceVersion(&version);

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(m_Context.PhysicalDevice, &props);

		KE_CORE_INFO("Vulkan Info");
		KE_CORE_INFO("	Version: {}.{}.{}", VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version), VK_API_VERSION_PATCH(version));
		KE_CORE_INFO("	Instance Version: {}", volkGetInstanceVersion());
		KE_CORE_INFO("	GLFW Version: {0}", (char*)glfwGetVersionString());
		KE_CORE_INFO("	Renderer: {0}", props.deviceName);
		KE_CORE_INFO("Initialized Vulkan!");
	}

	void VulkanContext::Init_Allocator()
	{
		VmaVulkanFunctions vma_vulkan_func{};
		vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorInfo = {};

		allocatorInfo.physicalDevice = m_Context.PhysicalDevice;
		allocatorInfo.device = m_Context.LogicalDevice;
		allocatorInfo.instance = m_Context.Instance;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
		allocatorInfo.pVulkanFunctions = &vma_vulkan_func;

		VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_Context.Allocator));
	}

	void VulkanContext::SetVSync(bool enabled)
	{
		
	}

	void VulkanContext::SwapBuffers()
	{
		
	}

	void VulkanContext::Cleanup()
	{
		vkQueueWaitIdle(m_Context.GraphicsQueue);
		vkDeviceWaitIdle(m_Context.LogicalDevice);

		KE_CORE_INFO("Cleanup Started!");

		ImGui_ImplVulkan_Shutdown();

		m_Context.Swapchain.Destroy(m_Context.LogicalDevice);

		vmaDestroyAllocator(m_Context.Allocator);

		while (m_DeviceDeletionQueue.size() > 0)
		{
			m_DeviceDeletionQueue.back()(volkGetLoadedDevice());
			m_DeviceDeletionQueue.pop_back();
		}

		while (m_InstanceDeletionQueue.size() > 0)
		{
			m_InstanceDeletionQueue.back()(m_Context.Instance);
			m_InstanceDeletionQueue.pop_back();
		}

		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}
}