#include "kepch.h"
#include "VulkanContext.h"

#define VOLK_IMPLEMENTATION
#include "volk.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

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

		Init_Instance();
		Init_Device();
		Init_Allocator();

		// Logging Vulkan Info
		uint32_t version;
		vkEnumerateInstanceVersion(&version);

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(GetVulkanVariables()->physicalDevice, &props);

		KE_CORE_INFO("Vulkan Info");
		KE_CORE_INFO("	Version: {}.{}.{}", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
		KE_CORE_INFO("	Instance Version: {}", volkGetInstanceVersion());
		KE_CORE_INFO("	GLFW Version: {0}", (char*)glfwGetVersionString());
		KE_CORE_INFO("	Renderer: {} {}", props.deviceName, props.vendorID);
		KE_CORE_INFO("Initialized Vulkan!");
	}

	void VulkanContext::Init_Instance()
	{
#pragma region Vulkan Instance Info

		uint32_t version;
		vkEnumerateInstanceVersion(&version);
		version &= ~(0xFFFU);

		VkApplicationInfo appInfo;
		appInfo.pApplicationName = glfwGetWindowTitle(m_WindowHandle);
		appInfo.apiVersion = version;
		appInfo.pEngineName = "Kairos Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pNext = NULL;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		VkInstanceCreateInfo info = {};

		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		info.enabledExtensionCount = glfwExtensionCount;
		info.ppEnabledExtensionNames = glfwExtensions;

#ifdef KE_DEBUG

		const char* layers[] = {
	"VK_LAYER_KHRONOS_validation"
		};

		info.enabledLayerCount = sizeof(layers) / sizeof(const char*);
		info.ppEnabledLayerNames = layers;

#endif // KE_DEBUG

#pragma endregion

		VK_CHECK(vkCreateInstance(&info, nullptr, &GetVulkanVariables()->instance));

		VkInstance handle = GetVulkanVariables()->instance;
		m_InstanceDeletionQueue.push_back([handle](VkInstance instance)
			{
				vkDestroyInstance(instance, nullptr);
				KE_CORE_INFO("Deleted Instance!");
			});

		volkLoadInstance(GetVulkanVariables()->instance);

		VkSurfaceKHR raw_surface = VK_NULL_HANDLE;
		glfwCreateWindowSurface(GetVulkanVariables()->instance, m_WindowHandle, nullptr, &raw_surface);
		GetVulkanVariables()->surface = raw_surface;

		m_InstanceDeletionQueue.push_back([this](VkInstance instance)
			{
				vkDestroySurfaceKHR(instance, GetVulkanVariables()->surface, nullptr);
				KE_CORE_INFO("Deleted Surface!");
			});
	}

	void VulkanContext::Init_Device()
	{
		SelectPhysicalDevice();
		CreateLogicalDevice();
	}

	void VulkanContext::SelectPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(GetVulkanVariables()->instance, &deviceCount, NULL);
		VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(deviceCount * sizeof(VkPhysicalDevice));
		vkEnumeratePhysicalDevices(GetVulkanVariables()->instance, &deviceCount, devices);

		// Score devices (Discrete GPU > Integrated > others)
		int bestScore = 0;

		for (uint32_t i = 0; i < deviceCount; i++)
		{
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(devices[i], &props);

			int score = 0;
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
			score += VK_VERSION_MAJOR(props.apiVersion) * 100 + VK_VERSION_MINOR(props.apiVersion);  // Prefer newer API Versions

			if (score > bestScore)
			{
				bestScore = score;
				GetVulkanVariables()->physicalDevice = devices[i];
			}
		}

		free(devices);

		if (GetVulkanVariables()->physicalDevice == VK_NULL_HANDLE)
		{
			KE_CORE_ERROR("No suitable GPU found");
			abort();
		}
	}

	void VulkanContext::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies();

		// Queue create infos
		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfos[2] = {};
		uint32_t queueCreateInfoCount = 0;

		// Graphics queue (may also support present)
		queueCreateInfos[queueCreateInfoCount++] = VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = indices.graphics_family,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		// Separate present queue if needed
		if (indices.present_family != indices.graphics_family) {
			queueCreateInfos[queueCreateInfoCount++] = VkDeviceQueueCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = indices.present_family,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			};
		}

		// Enable modern features via pNext chain
		VkPhysicalDeviceFeatures2 features2 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
		};
		VkPhysicalDeviceVulkan12Features features12 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.descriptorIndexing = VK_TRUE  // Enable bindless
		};
		features2.pNext = &features12;

		const char* extensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
		};

		VkDeviceCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features2,
			.queueCreateInfoCount = queueCreateInfoCount,
			.pQueueCreateInfos = queueCreateInfos,
			.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]),
			.ppEnabledExtensionNames = extensions
		};

// Validation layers (debug only)
#ifdef KE_DEBUG

		const char* layers[] = {
	"VK_LAYER_KHRONOS_validation"
		};

		createInfo.enabledLayerCount = sizeof(layers) / sizeof(const char*);
		createInfo.ppEnabledLayerNames = layers;

#endif // KE_DEBUG

		VK_CHECK(vkCreateDevice(GetVulkanVariables()->physicalDevice, &createInfo, NULL, &GetVulkanVariables()->device));

		VkDevice handle = GetVulkanVariables()->device;

		m_DeviceDeletionQueue.push_back([handle](VkDevice device)
			{
				vkDestroyDevice(device, nullptr);
				KE_CORE_INFO("Deleted Logical Device!");
			});

		// Get queues
		vkGetDeviceQueue(GetVulkanVariables()->device, indices.graphics_family, 0, &GetVulkanVariables()->graphicsQueue);
		vkGetDeviceQueue(GetVulkanVariables()->device, indices.present_family, 0, &GetVulkanVariables()->presentQueue);
		if (indices.has_compute) {
			vkGetDeviceQueue(GetVulkanVariables()->device, indices.compute_family, 0, &GetVulkanVariables()->computeQueue);
		}

		// Load device functions with Volk
		volkLoadDevice(GetVulkanVariables()->device);
	}

	QueueFamilyIndices VulkanContext::FindQueueFamilies()
	{
		QueueFamilyIndices indices = { 0 };

		indices.graphics_family = UINT32_MAX;
		indices.present_family = UINT32_MAX;
		indices.compute_family = UINT32_MAX;

		uint32_t queueFamilyCount = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(GetVulkanVariables()->physicalDevice, &queueFamilyCount, NULL);
		VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
		vkGetPhysicalDeviceQueueFamilyProperties(GetVulkanVariables()->physicalDevice, &queueFamilyCount, queueFamilies);

		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
			// Graphics queue
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphics_family = i;
			}

			// Present queue
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(GetVulkanVariables()->physicalDevice, i, GetVulkanVariables()->surface, &presentSupport);
			if (presentSupport) {
				indices.present_family = i;
			}

			// Dedicated compute queue (no graphics capability)
			if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
				!(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				indices.compute_family = i;
				indices.has_compute = true;
			}
		}

		free(queueFamilies);
		return indices;
	}

	void VulkanContext::Init_Allocator()
	{
		VmaVulkanFunctions vma_vulkan_func{};
		vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorInfo = {};

		allocatorInfo.physicalDevice = GetVulkanVariables()->physicalDevice;
		allocatorInfo.device = GetVulkanVariables()->device;
		allocatorInfo.instance = GetVulkanVariables()->instance;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
		allocatorInfo.pVulkanFunctions = &vma_vulkan_func;

		VK_CHECK(vmaCreateAllocator(&allocatorInfo, &GetVulkanVariables()->allocator));
	}

	void VulkanContext::SetVSync(bool enabled)
	{

	}

	void VulkanContext::SwapBuffers()
	{

	}

	void VulkanContext::Cleanup()
	{
		vkQueueWaitIdle(GetVulkanVariables()->graphicsQueue);
		KE_CORE_INFO("Cleanup Started!");

		vmaDestroyAllocator(GetVulkanVariables()->allocator);

		while (m_DeviceDeletionQueue.size() > 0)
		{
			m_DeviceDeletionQueue.back()(volkGetLoadedDevice());
			m_DeviceDeletionQueue.pop_back();
		}

		while (m_InstanceDeletionQueue.size() > 0)
		{
			m_InstanceDeletionQueue.back()(GetVulkanVariables()->instance);
			m_InstanceDeletionQueue.pop_back();
		}
	}
}