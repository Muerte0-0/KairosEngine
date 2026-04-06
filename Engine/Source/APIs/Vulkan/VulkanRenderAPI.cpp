#include "kepch.h"
#include "VulkanRenderAPI.h"
#include "Engine/Renderer/RHI/Shader.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "VulkanBuffer.h"

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

	void VulkanRenderAPI::Init(void* windowHandle, const std::filesystem::path& shaderDirectory)
	{
		m_ShaderDirectory = shaderDirectory;

		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddrPtr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(glfwGetInstanceProcAddress(nullptr, "vkGetInstanceProcAddr"));
    
		if (!vkGetInstanceProcAddrPtr)
		{
			throw runtime_error("Failed to get vkGetInstanceProcAddr");
		}
    
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

		CreateSquareMesh();
		CreateUniformBuffers();
		
		CreateViewportFramebuffer();
		CreateGraphicsPipeline();
	}

	void VulkanRenderAPI::BeginScene()
	{
		auto& commandBuffer = m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex];

		auto fenceResult = m_VulkanDevice->GetDevice().waitForFences(*m_VulkanCommand->GetInFlightFences()[m_CurrentFrameIndex], vk::True, UINT64_MAX);
		ASSERT(fenceResult == vk::Result::eSuccess, "Failed to wait for Fence!");

		ApplyPendingFramebufferResize();

		auto [result, imageIndex] = m_VulkanSwapchain->GetSwapChain().acquireNextImage(UINT64_MAX,
			*m_VulkanCommand->GetPresentCompleteSemaphores()[m_CurrentFrameIndex], nullptr);
		
		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			m_VulkanSwapchain->Recreate();
			CreateViewportFramebuffer();
			m_FrameValid = false;
			return;
		}

		ASSERT(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR, "Failed to acquire Swapchain Image!");
		m_CurrentImageIndex = imageIndex;

		UpdateUniformBuffer(m_CurrentFrameIndex);
		
		m_VulkanDevice->GetDevice().resetFences(*m_VulkanCommand->GetInFlightFences()[m_CurrentFrameIndex]);

		commandBuffer.reset();
		commandBuffer.begin({});
		m_FrameValid = true;
	}

	void VulkanRenderAPI::DrawFrame()
	{
		if (!m_FrameValid)
			return;
		
		auto& commandBuffer = m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex];

		VulkanFramebuffer* framebuffer = dynamic_cast<VulkanFramebuffer*>(m_ViewportFramebuffer.get());
		
		// Render the scene into the editor viewport image first.
		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			framebuffer->GetImage(),
			framebuffer->GetCurrentLayout(),
			vk::ImageLayout::eColorAttachmentOptimal,
			{},
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eTopOfPipe,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor);
		framebuffer->SetCurrentLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::ClearValue viewportClearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
		vk::ClearValue viewportClearDepth = vk::ClearDepthStencilValue(1.0f, 0);

		vk::RenderingAttachmentInfo viewportColorAttachment;
		viewportColorAttachment.imageView = framebuffer->GetImageView();
		viewportColorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		viewportColorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		viewportColorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		viewportColorAttachment.clearValue = viewportClearColor;
		
		vk::RenderingInfo viewportRenderingInfo;
		viewportRenderingInfo.renderArea = vk::Rect2D(vk::Offset2D(0, 0), framebuffer->GetExtent());
		viewportRenderingInfo.layerCount = 1;
		viewportRenderingInfo.colorAttachmentCount = 1;
		viewportRenderingInfo.pColorAttachments = &viewportColorAttachment;

		commandBuffer.beginRendering(viewportRenderingInfo);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *dynamic_cast<VulkanGraphicsPipeline*>(m_ViewportPipeline.get())->GetPipeline());
		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f,
			static_cast<float>(framebuffer->GetWidth()),
			static_cast<float>(framebuffer->GetHeight()),
			0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), framebuffer->GetExtent()));
		DrawMesh(commandBuffer, m_CubeMesh);
		commandBuffer.endRendering();

		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			framebuffer->GetImage(),
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::AccessFlagBits2::eShaderRead,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::ImageAspectFlagBits::eColor);
		framebuffer->SetCurrentLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		BeginSwapchainRendering(commandBuffer);
	}
	
	void VulkanRenderAPI::DrawMesh(vk::CommandBuffer commandBuffer, const Ref<Mesh>& mesh) const
	{
		// --- Vertex Buffer ---
		const auto& vb = mesh->GetVertexBuffer();
		auto vkVB = dynamic_cast<VulkanVertexBuffer*>(vb.get());
		vk::DeviceSize offsets[] = { 0 };

		commandBuffer.bindVertexBuffers(0, *vkVB->GetBuffer(), offsets);

		// --- Index Buffer ---
		const auto& ib = mesh->GetIndexBuffer();
		auto vkIB = dynamic_cast<VulkanIndexBuffer*>(ib.get());

		commandBuffer.bindIndexBuffer(*vkIB->GetBuffer(), 0, vk::IndexType::eUint32);

		// --- Uniform Buffer ---
		VulkanGraphicsPipeline* pipeline = dynamic_cast<VulkanGraphicsPipeline*>(m_ViewportPipeline.get());
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(),
			0, *pipeline->GetDescriptorSets()[m_CurrentFrameIndex], nullptr);
		
		// --- Draw ---
		commandBuffer.drawIndexed(
			vkIB->GetCount(), // Index Count
			1,                // Instance Count
			0,                // First Index
			0,                // Vertex Offset
			0                 // First Instance
		);
	}
	
	void VulkanRenderAPI::EndScene()
	{
		if (!m_FrameValid)
		{
			return;
		}
		
		auto& commandBuffer = m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex];
		
		commandBuffer.endRendering();
		
		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			m_VulkanSwapchain->GetSwapChainImages()[m_CurrentImageIndex],
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			{},
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eBottomOfPipe,
			vk::ImageAspectFlagBits::eColor);
		
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
			CreateViewportFramebuffer();
			CreateGraphicsPipeline();
		}
		else
		{
			assert(result == vk::Result::eSuccess);
		}
		
		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderAPI::WindowResized()
	{
		m_VulkanSwapchain->Recreate();
		m_VulkanCommand->RecreatePresentSemaphores(static_cast<uint32_t>(m_VulkanSwapchain->GetSwapChainImages().size()));
		CreateViewportFramebuffer();
	}

	void VulkanRenderAPI::ResizeFramebuffer(uint32_t width, uint32_t height)
	{
		if (m_ViewportFramebuffer == nullptr)
			return;

		width = (std::max)(width, 1u);
		height = (std::max)(height, 1u);

		if (m_ViewportFramebuffer->GetWidth() == width && m_ViewportFramebuffer->GetHeight() == height)
		{
			return;
		}

		m_PendingViewportFramebufferSpecification.Width = width;
		m_PendingViewportFramebufferSpecification.Height = height;
		m_ViewportFramebufferResizePending = true;
	}

	void VulkanRenderAPI::ApplyPendingFramebufferResize()
	{
		if (!m_ViewportFramebufferResizePending || m_ViewportFramebuffer == nullptr)
		{
			return;
		}

		// The viewport image may still be referenced by older in-flight frames and ImGui draw data,
		// so resize only after the device is fully idle.
		m_VulkanDevice->WaitIdle();
		m_ViewportFramebuffer->Resize(
			m_PendingViewportFramebufferSpecification.Width,
			m_PendingViewportFramebufferSpecification.Height);
		m_ViewportFramebufferResizePending = false;
	}

	void VulkanRenderAPI::CreateGraphicsPipeline()
	{
		// Load (or retrieve from cache) the viewport shader via the library.
		Ref<Shader> meshShader = m_ShaderLibrary.Load(ShaderDescriptor{
			.Name            = "Mesh",
			.ShaderDirectory = m_ShaderDirectory,
			.Stages = {
				{.Stage = ShaderStage::Vertex, .Filepath = "Mesh.vertMain.vert.spv", .EntryPoint = "vertMain" },
				{.Stage = ShaderStage::Fragment, .Filepath = "Mesh.fragMain.frag.spv", .EntryPoint = "fragMain" },
			}
		});

		std::vector<Ref<Mesh>> meshes;
		meshes.push_back(m_CubeMesh);
		
		GraphicsPipelineCreateInfo createInfo;
		createInfo.Shader		= meshShader;
		createInfo.ColorFormat	= VulkanUtils::ToTextureFormat(m_VulkanSwapchain->GetSwapChainImageFormat());
		createInfo.DepthFormat	= VulkanUtils::ToTextureFormat(m_VulkanSwapchain->GetDepthImageFormat());
		createInfo.SampleCount	= VulkanUtils::ToSampleCountBits(vk::SampleCountFlagBits::e1);
		createInfo.Meshes		= meshes;

		// GraphicsPipeline::Create calls Init() internally.
		m_ViewportPipeline = GraphicsPipeline::Create(std::move(createInfo));
	}

	void VulkanRenderAPI::CreateUniformBuffers()
	{
		m_UniformBuffers.clear();
		m_UniformBuffersMemory.clear();
		m_UniformBuffersMapped.clear();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
			vk::raii::Buffer buffer({});
			vk::raii::DeviceMemory bufferMem({});
			
			VulkanUtils::CreateBuffer(m_VulkanDevice->GetDevice(), m_VulkanDevice->GetPhysicalDevice(),
				bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				buffer, bufferMem);
			
			m_UniformBuffers.emplace_back(std::move(buffer));
			m_UniformBuffersMemory.emplace_back(std::move(bufferMem));
			m_UniformBuffersMapped.emplace_back(m_UniformBuffersMemory[i].mapMemory(0, bufferSize));
		}
	}

	void VulkanRenderAPI::UpdateUniformBuffer(uint32_t currentImage) const
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto  currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		
		ubo.Model = rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//ubo.Model = glm::mat4(1.0f);
		
		ubo.View = lookAt(
			glm::vec3(0.0f, -2.0f, 4.0f),		// Camera position
			glm::vec3(0.0f, 0.0f, 0.0f),		// Look at Origin
			glm::vec3(0.0f, 1.0f, 0.0f)		// Up Vector
			);
		
		ubo.Proj = glm::perspective(glm::radians(45.0f),
			static_cast<float>(m_ViewportFramebuffer->GetWidth()) / static_cast<float>(m_ViewportFramebuffer->GetHeight()),
			0.1f, 100.0f);
		
		ubo.Proj[1][1] *= -1;

		memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	void VulkanRenderAPI::CreateViewportFramebuffer()
	{
		uint32_t width = (std::max)(m_VulkanSwapchain->GetSwapChainExtent().width, 1u);
		uint32_t height = (std::max)(m_VulkanSwapchain->GetSwapChainExtent().height, 1u);
		
		FramebufferSpecification spec{
			.Width = width,
			.Height = height,
			.Format = VulkanUtils::ToTextureFormat(m_VulkanSwapchain->GetSwapChainImageFormat()),
			.SampleBits = SampleCountBits::s1
		};
		
		m_ViewportFramebuffer = Framebuffer::Create(spec);
	}

	void VulkanRenderAPI::BeginSwapchainRendering(vk::CommandBuffer commandBuffer) const
	{
		// Transition Swapchain targets into Renderable layouts for the ImGui pass.
		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			m_VulkanSwapchain->GetSwapChainImages()[m_CurrentImageIndex],
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eTopOfPipe,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor);

		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			*m_VulkanSwapchain->GetColorImage(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eTopOfPipe,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor);

		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			*m_VulkanSwapchain->GetDepthImage(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthAttachmentOptimal,
			{},
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eTopOfPipe,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::ImageAspectFlagBits::eDepth);

		commandBuffer.beginRendering(m_VulkanSwapchain->GetRenderingInfo(m_CurrentImageIndex));
	}
	
	void VulkanRenderAPI::CreateSquareMesh()
	{
		std::vector<Vertex> vertices =
		{
			// Front face (Z+)
			{.Position = {-0.5f, -0.5f,  0.5f}, .Color = {0.5f, 0, 0}},
			{.Position = { 0.5f, -0.5f,  0.5f}, .Color = {0, 0.5f,0}},
			{.Position = { 0.5f,  0.5f,  0.5f}, .Color = {0, 0, 0.5f}},
			{.Position = {-0.5f,  0.5f,  0.5f}, .Color = {0.5f, 0.5f, 0}},

			// Back face (Z-)
			{.Position = {-0.5f, -0.5f, -0.5f}, .Color = {0.5f, 0, 0.5f}},
			{.Position = { 0.5f, -0.5f, -0.5f}, .Color = {0, 0.5f, 0.5f}},
			{.Position = { 0.5f,  0.5f, -0.5f}, .Color = {0.5f, 0, 0.5f}},
			{.Position = {-0.5f,  0.5f, -0.5f}, .Color = {0, 0.5f, 0}},
		};
		
		Ref<VertexBuffer> vb = VertexBuffer::Create(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(Vertex)));

		BufferLayout layout =
		{
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Color" }
		};

		vb->SetLayout(layout);
		
		std::vector<uint32_t> indices =
		{
			// Front
			0, 1, 2,
			2, 3, 0,

			// Back
			5, 4, 7,
			7, 6, 5,

			// Left
			4, 0, 3,
			3, 7, 4,

			// Right
			1, 5, 6,
			6, 2, 1,

			// Top
			3, 2, 6,
			6, 7, 3,

			// Bottom
			4, 5, 1,
			1, 0, 4
		};

		Ref<IndexBuffer> ib = IndexBuffer::Create( indices.data(), static_cast<uint32_t>(indices.size()));

		m_CubeMesh = CreateRef<Mesh>(vb, ib);
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
		
		std::vector<char const*> requiredLayers;
		if (enableValidationLayers)
		{
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());
		}
		
		auto layerProperties = m_Context.enumerateInstanceLayerProperties();
		auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
			[&layerProperties](auto const& requiredLayer)
			{
				return std::ranges::none_of(layerProperties,
					[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
			});
		
		ASSERT(unsupportedLayerIt == requiredLayers.end(), "Required layer not supported: {}", std::string(*unsupportedLayerIt));

		auto requiredExtensions = GetRequiredInstanceExtensions();
		auto extensionProperties = m_Context.enumerateInstanceExtensionProperties();
		auto unsupportedPropertyIt = std::ranges::find_if(requiredExtensions,
			[&extensionProperties](auto const& requiredExtension)
			{
				return std::ranges::none_of(extensionProperties,
					[requiredExtension](auto const& extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
			});
		
		ASSERT(unsupportedPropertyIt == requiredExtensions.end(), "Required extension not supported: {}", std::string(*unsupportedPropertyIt));

		vk::InstanceCreateInfo createInfo;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
		createInfo.ppEnabledLayerNames = requiredLayers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		m_Instance = vk::raii::Instance(m_Context, createInfo);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Instance);
	}
	
	vector<const char*> VulkanRenderAPI::GetRequiredInstanceExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(vk::EXTDebugUtilsExtensionName);
		}

		return extensions;
	}

	void VulkanRenderAPI::SetupDebugMessenger()
	{
		if (!enableValidationLayers)
			return;

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
		} catch (vk::SystemError& err)
		{
			LOG(LogLevel::Error, "Debug messenger not available. Validation layers may not be enabled. {}", err.what());
		}
	}

	void VulkanRenderAPI::CreateSurface(void* windowHandle)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkResult result = glfwCreateWindowSurface(*m_Instance, static_cast<GLFWwindow*>(windowHandle), nullptr, &surface);
		ASSERT(result == VK_SUCCESS, "Failed to create window surface: {}", glfwGetErrorName(result));

		m_Surface = vk::raii::SurfaceKHR(m_Instance, surface);
	}
}
