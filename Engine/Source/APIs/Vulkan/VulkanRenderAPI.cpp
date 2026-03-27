#include "kepch.h"
#include "VulkanRenderAPI.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>

#include "VulkanUtils.h"

namespace Engine
{
	const std::vector<char const*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char *> requiredDeviceExtensions = {
		vk::KHRSwapchainExtensionName,
		vk::KHRSpirv14ExtensionName,
		vk::KHRSynchronization2ExtensionName
	};
	
#ifdef NDEBUG
	constexpr bool enableValidationLayers = false;
#else
	constexpr bool enableValidationLayers = true;
#endif
	
	constexpr int MAX_FRAMES_IN_FLIGHT = 3;

	void VulkanRenderAPI::Init(void* windowHandle)
	{
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddrPtr = 
		reinterpret_cast<PFN_vkGetInstanceProcAddr>(
			glfwGetInstanceProcAddress(nullptr, "vkGetInstanceProcAddr")
		);
    
		if (!vkGetInstanceProcAddrPtr)
			throw runtime_error("Failed to get vkGetInstanceProcAddr");
    
		// Initialize the global dispatcher
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddrPtr);
		
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface(windowHandle);
		
		m_VulkanDevice = CreateScope<VulkanDevice>(m_Instance, m_Surface, requiredDeviceExtensions);
		
		m_VulkanDevice->PickPhysicalDevice();
		m_VulkanDevice->CreateLogicalDevice(enableValidationLayers, validationLayers);
		
		m_VulkanSwapchain = CreateScope<VulkanSwapchain>(*m_VulkanDevice, static_cast<GLFWwindow*>(windowHandle));
		
		m_VulkanSwapchain->Create();
		m_VulkanSwapchain->CreateImageViews();
		
		m_VulkanSwapchain->CreateColorResources();
		m_VulkanSwapchain->CreateDepthResources();
		m_VulkanSwapchain->SetupDynamicRendering();
		
		m_VulkanCommand = CreateScope<VulkanCommand>(*m_VulkanDevice);
		
		m_VulkanCommand->CreateCommandPool();
		m_VulkanCommand->CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
		m_VulkanCommand->CreateSyncObjects(static_cast<uint32_t>(m_VulkanSwapchain->GetSwapChainImages().size()), MAX_FRAMES_IN_FLIGHT);
	}

	void VulkanRenderAPI::BeginFrame()
	{
		auto& commandBuffer = m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex];

		auto fenceResult = m_VulkanDevice->GetDevice().waitForFences(*m_VulkanCommand->GetInFlightFences()[m_CurrentFrameIndex], vk::True, UINT64_MAX);

		KE_CORE_ASSERT(fenceResult != vk::Result::eSuccess, "Failed to wait for Fence!")

		auto [result, imageIndex] = m_VulkanSwapchain->GetSwapChain().acquireNextImage(UINT64_MAX,
			*m_VulkanCommand->GetPresentCompleteSemaphores()[m_CurrentFrameIndex], nullptr);
		
		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			m_VulkanSwapchain->Recreate();
			m_FrameValid = false;
			return;
		}

		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		{
			KE_CORE_ASSERT((vk::Result::eTimeout || result == vk::Result::eNotReady), "Failed to acquire Swapchain Image!")
		}
		
		if (result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR)
			m_CurrentImageIndex = imageIndex;

		//UpdateUniformBuffer(m_CurrentFrameIndex);

		m_VulkanDevice->GetDevice().resetFences(*m_VulkanCommand->GetInFlightFences()[m_CurrentFrameIndex]);

		commandBuffer.reset();
		
		commandBuffer.begin({});
		
		// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			m_VulkanSwapchain->GetSwapChainImages()[m_CurrentImageIndex],
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},															// srcAccessMask (no need to wait for previous operations)
			vk::AccessFlagBits2::eColorAttachmentWrite,					// dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,			// srcStage
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,			// dstStage
			vk::ImageAspectFlagBits::eColor
		);
		
		// Transition the multisampled color image to COLOR_ATTACHMENT_OPTIMAL
		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			*m_VulkanSwapchain->GetColorImage(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::AccessFlagBits2::eColorAttachmentWrite,					// srcAccessMask
			vk::AccessFlagBits2::eColorAttachmentWrite,					// dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,			// srcStage
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,			// dstStage
			vk::ImageAspectFlagBits::eColor
		);

		// Transition the depth image to DEPTH_ATTACHMENT_OPTIMAL
		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			*m_VulkanSwapchain->GetDepthImage(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthAttachmentOptimal,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::ImageAspectFlagBits::eDepth);
		
		commandBuffer.beginRendering(m_VulkanSwapchain->GetRenderingInfo(m_CurrentImageIndex));
		
		m_FrameValid = true;
	}

	void VulkanRenderAPI::DrawFrame()
	{
		if (!m_FrameValid) return;
		
		auto& commandBuffer = m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex];
		
		//commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, GetGraphicsPipeline());
		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f,
			static_cast<float>(m_VulkanSwapchain->GetSwapChainExtent().width),
			static_cast<float>(m_VulkanSwapchain->GetSwapChainExtent().height), 0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_VulkanSwapchain->GetSwapChainExtent()));
		//commandBuffer.bindVertexBuffers(0, *vertexBuffer, { 0 });
		//commandBuffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint32);
		//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[frameIndex], nullptr);
		//commandBuffer.draw(1, 0, 0, 0);
	}
	
	void VulkanRenderAPI::EndFrame()
	{
		if (!m_FrameValid) return;
		
		auto& commandBuffer = m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex];
		
		commandBuffer.endRendering();
		
		// After rendering, transition the swapchain image to PRESENT_SRC
		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			m_VulkanSwapchain->GetSwapChainImages()[m_CurrentImageIndex],
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,					// srcAccessMask
			{},															// dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,			// srcStage
			vk::PipelineStageFlagBits2::eBottomOfPipe,					// dstStage
			vk::ImageAspectFlagBits::eColor
		);
		
		commandBuffer.end();
		
		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

		vk::SubmitInfo submitInfo;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &*m_VulkanCommand->GetPresentCompleteSemaphores()[m_CurrentFrameIndex];
		submitInfo.pWaitDstStageMask = &waitDestinationStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &*commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &*m_VulkanCommand->GetRenderFinishedSemaphores()[m_CurrentFrameIndex];

		m_VulkanDevice->GetGraphicsQueue().submit(submitInfo, *m_VulkanCommand->GetInFlightFences()[m_CurrentFrameIndex]);

		vk::PresentInfoKHR presentInfoKHR;
		presentInfoKHR.waitSemaphoreCount = 1;
		presentInfoKHR.pWaitSemaphores = &*m_VulkanCommand->GetRenderFinishedSemaphores()[m_CurrentFrameIndex];
		presentInfoKHR.swapchainCount = 1;
		presentInfoKHR.pSwapchains = &*m_VulkanSwapchain->GetSwapChain();
		presentInfoKHR.pImageIndices = &m_CurrentImageIndex;

		auto result = m_VulkanDevice->GetGraphicsQueue().presentKHR(presentInfoKHR);

		if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || m_FramebufferResized)
		{
			m_FramebufferResized = false;
			m_VulkanSwapchain->Recreate();
		}
		else
			// There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
			assert(result == vk::Result::eSuccess);
		
		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderAPI::WindowResized()
	{
		m_VulkanSwapchain->Recreate();
		m_VulkanCommand->RecreatePresentSemaphores(static_cast<uint32_t>(m_VulkanSwapchain->GetSwapChainImages().size()));
	}

	void VulkanRenderAPI::CreateInstance()
	{
		vk::ApplicationInfo appInfo{
			"Engine",
			VK_MAKE_VERSION(1, 0, 0),
			"No Engine",
			VK_MAKE_VERSION(1, 0, 0)
		};
		
		appInfo.apiVersion = VK_API_VERSION_1_3;
		
		// Get the required layers
		std::vector<char const*> requiredLayers;
		if (enableValidationLayers)
		{
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());
		}
		
		// Check if the required layers are supported by the Vulkan implementation.
		auto layerProperties = m_Context.enumerateInstanceLayerProperties();
		auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
			[&layerProperties](auto const& requiredLayer) {
				return std::ranges::none_of(layerProperties,
					[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
			});
		
		KE_CORE_ASSERT(unsupportedLayerIt != requiredLayers.end(), "Required layer not supported: {}", std::string(*unsupportedLayerIt))

		// Get the required extensions.
		auto requiredExtensions = GetRequiredInstanceExtensions();

		// Check if the required extensions are supported by the Vulkan implementation.
		auto extensionProperties = m_Context.enumerateInstanceExtensionProperties();
		auto unsupportedPropertyIt =
			std::ranges::find_if(requiredExtensions,
				[&extensionProperties](auto const& requiredExtension) {
					return std::ranges::none_of(extensionProperties,
						[requiredExtension](auto const& extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
				});
		
		if (unsupportedPropertyIt != requiredExtensions.end())
		{
			KE_CORE_ASSERT(false, "Required extension not supported: {}", std::string(*unsupportedPropertyIt))
		}

		vk::InstanceCreateInfo createInfo;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
		createInfo.ppEnabledLayerNames = requiredLayers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		m_Instance = vk::raii::Instance(m_Context, createInfo);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Instance);
	}
	
	vector<const char*> VulkanRenderAPI::GetRequiredInstanceExtensions() const
	{
		uint32_t glfwExtensionCount = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
			extensions.push_back(vk::EXTDebugUtilsExtensionName);

		return extensions;
	}

	void VulkanRenderAPI::SetupDebugMessenger()
	{
		if (!enableValidationLayers) return;

		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT;
		debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags;
		debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags;
		debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &DebugCallback;

		try
		{
			m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
		}
		catch (vk::SystemError& err)
		{
			LOG(LogLevel::Error, "Debug messenger not available. Validation layers may not be enabled. {}", err.what());
		}
	}

	void VulkanRenderAPI::CreateSurface(void* windowHandle)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkResult result = glfwCreateWindowSurface(*m_Instance, static_cast<GLFWwindow*>(windowHandle), nullptr, &surface);

		KE_CORE_ASSERT(result == VK_SUCCESS, "Failed to create window surface: {}", glfwGetErrorName(result));

		m_Surface = vk::raii::SurfaceKHR(m_Instance, surface);
	}
}
