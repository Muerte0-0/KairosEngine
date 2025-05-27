#include "kepch.h"
#include "VulkanContext.h"

#define VOLK_IMPLEMENTATION
#include "volk.h"

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

		VkInstance instance;
		VkInstanceCreateInfo info = {};

#ifdef KE_DEBUG

		const char* layers[] = {
	"VK_LAYER_KHRONOS_validation"
		};

		info.enabledLayerCount = 1;
		info.ppEnabledLayerNames = layers;

#endif // KE_DEBUG


		vkCreateInstance(&info, nullptr, &instance);
		volkLoadInstance(instance);

		uint32_t version;
		vkEnumerateInstanceVersion(&version);

		KE_CORE_INFO("Vulkan Info");
		KE_CORE_INFO("	Version: {}.{}.{}", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
		KE_CORE_INFO("	Instance Version: {}", volkGetInstanceVersion());
		KE_CORE_INFO("	GLFW Version: {0}", (char*)glfwGetVersionString());
		KE_CORE_INFO("Initialized Vulkan!");
	}

	void VulkanContext::SetVSync(bool enabled)
	{

	}

	void VulkanContext::SwapBuffers()
	{

	}

	void VulkanContext::Cleanup()
	{

	}
}