#include "kepch.h"
#include "VulkanRenderAPI.h"
#include "Engine/Renderer/RHI/Shader.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"
#include "VulkanBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanGraphicsPipeline.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>

#include "VulkanUtils.h"
#include "VulkanMaterial.h"
#include "Engine/Utils/RendererUtils.h"

namespace Engine
{
	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> requiredDeviceExtensions =
	{
		vk::KHRSwapchainExtensionName,
		vk::KHRSpirv14ExtensionName,
		vk::KHRSynchronization2ExtensionName
	};

#ifdef NDEBUG
	constexpr bool enableValidationLayers = false;
#else
	constexpr bool enableValidationLayers = true;
#endif

	// -----------------------------------------------------------------------
	// Init
	// -----------------------------------------------------------------------

	void VulkanRenderAPI::Init(void* windowHandle, const std::filesystem::path& shaderDirectory)
	{
		m_ShaderDirectory = shaderDirectory;

		auto vkGetInstanceProcAddrPtr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
			glfwGetInstanceProcAddress(nullptr, "vkGetInstanceProcAddr"));
		if (!vkGetInstanceProcAddrPtr)
			throw std::runtime_error("Failed to get vkGetInstanceProcAddr");

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

		m_VulkanCommand = CreateScope<VulkanCommand>(*m_VulkanDevice);
		m_VulkanCommand->CreateCommandPool();
		m_VulkanCommand->CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
		m_VulkanCommand->CreateSyncObjects(
			static_cast<uint32_t>(m_VulkanSwapchain->GetSwapChainImages().size()),
			MAX_FRAMES_IN_FLIGHT);

		CreateUniformBuffers();
		CreateMaterialDescriptorSetLayout();
	}

	// -----------------------------------------------------------------------
	// Frame lifecycle
	// -----------------------------------------------------------------------

	void VulkanRenderAPI::BeginScene()
	{
		auto& commandBuffer = m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex];

		auto fenceResult = m_VulkanDevice->GetDevice().waitForFences(
			*m_VulkanCommand->GetInFlightFences()[m_CurrentFrameIndex], vk::True, UINT64_MAX);
		ASSERT(fenceResult == vk::Result::eSuccess, "Failed to wait for Fence!");

		auto [result, imageIndex] = m_VulkanSwapchain->GetSwapChain().acquireNextImage(
			UINT64_MAX,
			*m_VulkanCommand->GetPresentCompleteSemaphores()[m_CurrentFrameIndex],
			nullptr);

		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			m_VulkanSwapchain->Recreate();
			m_FrameValid = false;
			return;
		}

		ASSERT(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR,
			"Failed to acquire Swapchain Image!");

		m_CurrentImageIndex = imageIndex;
		m_VulkanDevice->GetDevice().resetFences(
			*m_VulkanCommand->GetInFlightFences()[m_CurrentFrameIndex]);

		commandBuffer.reset();
		commandBuffer.begin({});
		m_FrameValid = true;
	}

	void VulkanRenderAPI::DrawFrame()
	{
		if (!m_FrameValid)
			return;

		// SceneRenderer has already recorded mesh draw calls into the active command buffer.
		// All that remains here is to open the swapchain pass for ImGui.
		BeginSwapchainRendering(*m_VulkanCommand->GetCommandBuffers()[m_CurrentFrameIndex]);
	}

	void VulkanRenderAPI::BeginPass(const RenderPass& renderPass)
	{
		if (!m_FrameValid || renderPass.TargetFramebuffer == nullptr)
			return;

		auto* framebuffer = dynamic_cast<VulkanFramebuffer*>(renderPass.TargetFramebuffer);
		ASSERT(framebuffer, "VulkanRenderAPI::BeginPass expects a VulkanFramebuffer.");
		m_ActiveFramebuffer   = framebuffer;
		m_OffscreenPassActive = true;

		vk::CommandBuffer commandBuffer = *GetActiveCommandBuffer();

		// Transition the resolve target (always needed — this is what gets sampled by ImGui).
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

		// When MSAA is active, the render target is the MSAA image, not the resolve image.
		// It also starts in eUndefined each frame and must be transitioned before beginRendering.
		if (framebuffer->HasMSAA())
		{
			VulkanUtils::TransitionImageLayout(
				commandBuffer,
				framebuffer->GetMSAAImage(),
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				{},
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eTopOfPipe,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::ImageAspectFlagBits::eColor);
		}

		if (framebuffer->HasDepth())
		{
			VulkanUtils::TransitionImageLayout(
				commandBuffer,
				framebuffer->GetDepthImage(),
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal,
				{},
				vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::PipelineStageFlagBits2::eTopOfPipe,
				vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				vk::ImageAspectFlagBits::eDepth);
		}

		vk::RenderingInfo renderingInfo = framebuffer->BuildRenderingInfo(renderPass.ClearColor);
		commandBuffer.beginRendering(renderingInfo);
	}

	void VulkanRenderAPI::EndPass()
	{
		if (!m_FrameValid || !m_OffscreenPassActive)
			return;

		vk::CommandBuffer commandBuffer = *GetActiveCommandBuffer();
		commandBuffer.endRendering();

		VulkanUtils::TransitionImageLayout(
			commandBuffer,
			m_ActiveFramebuffer->GetImage(),
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::AccessFlagBits2::eShaderRead,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::ImageAspectFlagBits::eColor);
		m_ActiveFramebuffer->SetCurrentLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		m_ActiveFramebuffer = nullptr;
		m_OffscreenPassActive = false;
	}

	void VulkanRenderAPI::DrawMesh(
		const Framebuffer& framebuffer,
		const GraphicsPipeline& pipeline,
		const Mesh& mesh,
		const glm::mat4& modelTransform,
		const UniformBufferObject& uniformBufferObject,
		const std::vector<Ref<Material>>& materials)
	{
		if (!m_FrameValid)
			return;

		const auto* vkFramebuffer = dynamic_cast<const VulkanFramebuffer*>(&framebuffer);
		const auto* vkPipeline = dynamic_cast<const VulkanGraphicsPipeline*>(&pipeline);
		const auto* vertexBuffer = dynamic_cast<const VulkanVertexBuffer*>(mesh.GetVertexBuffer().get());
		const auto* indexBuffer = dynamic_cast<const VulkanIndexBuffer*>(mesh.GetIndexBuffer().get());
		ASSERT(vkFramebuffer, "VulkanRenderAPI::DrawMesh expects a VulkanFramebuffer.");
		ASSERT(vkPipeline, "VulkanRenderAPI::DrawMesh expects a VulkanGraphicsPipeline.");
		ASSERT(vertexBuffer, "VulkanRenderAPI::DrawMesh expects a VulkanVertexBuffer.");
		ASSERT(indexBuffer, "VulkanRenderAPI::DrawMesh expects a VulkanIndexBuffer.");
		
		vk::CommandBuffer commandBuffer = *GetActiveCommandBuffer();
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *vkPipeline->GetPipeline());
		commandBuffer.setViewport(
			0,
			vk::Viewport(
				0.0f,
				0.0f,
				static_cast<float>(vkFramebuffer->GetWidth()),
				static_cast<float>(vkFramebuffer->GetHeight()),
				0.0f,
				1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), vkFramebuffer->GetExtent()));

		memcpy(
			m_UniformBuffersMapped[m_CurrentFrameIndex],
			&uniformBufferObject,
			sizeof(uniformBufferObject));

		const PushConstantObject pushConstantObject{
			.Model = modelTransform
		};

		vk::DeviceSize offset = 0;
		commandBuffer.bindVertexBuffers(0, *vertexBuffer->GetBuffer(), offset);
		commandBuffer.bindIndexBuffer(*indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
		commandBuffer.pushConstants(
			*vkPipeline->GetPipelineLayout(),
			vk::ShaderStageFlagBits::eVertex,
			0,
			sizeof(PushConstantObject),
			&pushConstantObject);
		commandBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			*vkPipeline->GetPipelineLayout(),
			0,
			*vkPipeline->GetDescriptorSets()[m_CurrentFrameIndex],
			nullptr);

		// Draw each SubMesh — bind its material (set 1) before the draw call.
		const auto& subMeshes = mesh.GetSubMeshes();
		if (subMeshes.empty())
		{
			// Fallback: bind default material for the whole mesh
			Ref<Material> mat = Material::GetDefault();
			vk::PipelineLayout layout = *vkPipeline->GetPipelineLayout();
			mat->Bind(&commandBuffer, &layout, m_CurrentFrameIndex);
			commandBuffer.drawIndexed(indexBuffer->GetCount(), 1, 0, 0, 0);
		}
		else
		{
			for (const SubMesh& sub : subMeshes)
			{
				// Resolve material for this submesh
				Ref<Material> mat = (sub.MaterialIndex < materials.size() && materials[sub.MaterialIndex])
					? materials[sub.MaterialIndex]
					: Material::GetDefault();

				vk::PipelineLayout layout = *vkPipeline->GetPipelineLayout();
				mat->Bind(&commandBuffer, &layout, m_CurrentFrameIndex);

				commandBuffer.drawIndexed(
					sub.IndexCount,
					1,
					sub.BaseIndex,
					static_cast<int32_t>(sub.BaseVertex),
					0);
			}
		}
	}

	void VulkanRenderAPI::EndScene()
	{
		if (!m_FrameValid)
			return;

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

		vk::PipelineStageFlags waitStage(vk::PipelineStageFlagBits::eColorAttachmentOutput);

		vk::SubmitInfo submitInfo;
		submitInfo.waitSemaphoreCount   = 1;
		submitInfo.pWaitSemaphores      = &*m_VulkanCommand->GetPresentCompleteSemaphores()[m_CurrentFrameIndex];
		submitInfo.pWaitDstStageMask    = &waitStage;
		submitInfo.commandBufferCount   = 1;
		submitInfo.pCommandBuffers      = &*commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores    = &*m_VulkanCommand->GetRenderFinishedSemaphores()[m_CurrentFrameIndex];

		m_VulkanDevice->GetGraphicsQueue().submit(
			submitInfo,
			*m_VulkanCommand->GetInFlightFences()[m_CurrentFrameIndex]);

		vk::PresentInfoKHR presentInfo;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores    = &*m_VulkanCommand->GetRenderFinishedSemaphores()[m_CurrentFrameIndex];
		presentInfo.swapchainCount     = 1;
		presentInfo.pSwapchains        = &*m_VulkanSwapchain->GetSwapChain();
		presentInfo.pImageIndices      = &m_CurrentImageIndex;

		auto result = m_VulkanDevice->GetGraphicsQueue().presentKHR(presentInfo);

		if (result == vk::Result::eSuboptimalKHR ||
			result == vk::Result::eErrorOutOfDateKHR ||
			m_SwapchainDirty)
		{
			m_SwapchainDirty = false;
			m_VulkanSwapchain->Recreate();
		}
		else
		{
			ASSERT(result == vk::Result::eSuccess, "presentKHR failed.");
		}

		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderAPI::WindowResized()
	{
		m_VulkanSwapchain->Recreate();
		m_VulkanCommand->RecreatePresentSemaphores(
			static_cast<uint32_t>(m_VulkanSwapchain->GetSwapChainImages().size()));
	}

	TextureFormat VulkanRenderAPI::GetDefaultColorFormat() const
	{
		return VulkanUtils::ToTextureFormat(m_VulkanSwapchain->GetSwapChainImageFormat());
	}

	TextureFormat VulkanRenderAPI::GetDefaultDepthFormat() const
	{
		const vk::Format fmt = VulkanUtils::FindSupportedFormat(
			m_VulkanDevice->GetPhysicalDevice(),
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment);
		return VulkanUtils::ToTextureFormat(fmt);
	}

	void VulkanRenderAPI::WaitIdle()
	{
		m_VulkanDevice->WaitIdle();
	}

	void VulkanRenderAPI::ReleaseStaticResources()
	{
		// Drop all static/shared GPU objects that live outside the normal
		// RAII member chain — must happen before WaitIdle() + device destroy.
		VulkanMaterial::ResetStaticResources();
	}

	void VulkanRenderAPI::CreateUniformBuffers()
	{
		m_UniformBuffers.clear();
		m_UniformBuffersMemory.clear();
		m_UniformBuffersMapped.clear();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
			vk::raii::Buffer       buffer({});
			vk::raii::DeviceMemory bufferMem({});

			VulkanUtils::CreateBuffer(
				m_VulkanDevice->GetDevice(),
				m_VulkanDevice->GetPhysicalDevice(),
				bufferSize,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				buffer, bufferMem);

			m_UniformBuffers.emplace_back(std::move(buffer));
			m_UniformBuffersMemory.emplace_back(std::move(bufferMem));
			m_UniformBuffersMapped.emplace_back(m_UniformBuffersMemory[i].mapMemory(0, bufferSize));
		}
	}

	// -----------------------------------------------------------------------
	// Swapchain rendering (ImGui pass)
	// -----------------------------------------------------------------------

	void VulkanRenderAPI::BeginSwapchainRendering(vk::CommandBuffer commandBuffer) const
	{
		// Transition the swapchain image to a color attachment for the ImGui pass.
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

		// Simple color-only attachment — ImGui needs no depth and no MSAA.
		vk::RenderingAttachmentInfo colorAttachment;
		colorAttachment.imageView   = *m_VulkanSwapchain->GetSwapChainImageViews()[m_CurrentImageIndex];
		colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colorAttachment.loadOp      = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp     = vk::AttachmentStoreOp::eStore;
		colorAttachment.clearValue  = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});

		vk::RenderingInfo renderInfo;
		renderInfo.renderArea           = vk::Rect2D(vk::Offset2D(0, 0), m_VulkanSwapchain->GetSwapChainExtent());
		renderInfo.layerCount           = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments    = &colorAttachment;
		renderInfo.pDepthAttachment     = nullptr;

		commandBuffer.beginRendering(renderInfo);
	}

	// -----------------------------------------------------------------------
	// Instance / surface / debug
	// -----------------------------------------------------------------------

	void VulkanRenderAPI::CreateInstance()
	{
		vk::ApplicationInfo appInfo;
		appInfo.pApplicationName   = "Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName        = "KairosEngine";
		appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion         = VK_API_VERSION_1_3;

		std::vector<const char*> requiredLayers;
		if (enableValidationLayers)
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());

		auto layerProperties = m_Context.enumerateInstanceLayerProperties();
		auto unsupportedLayer = std::ranges::find_if(requiredLayers,
			[&](const char* required)
			{
				return std::ranges::none_of(layerProperties,
					[required](const auto& prop) { return strcmp(prop.layerName, required) == 0; });
			});
		ASSERT(unsupportedLayer == requiredLayers.end(),
			"Required layer not supported: {}", std::string(*unsupportedLayer));

		auto requiredExtensions = GetRequiredInstanceExtensions();
		auto extensionProperties = m_Context.enumerateInstanceExtensionProperties();
		auto unsupportedExt = std::ranges::find_if(requiredExtensions,
			[&](const char* required)
			{
				return std::ranges::none_of(extensionProperties,
					[required](const auto& prop) { return strcmp(prop.extensionName, required) == 0; });
			});
		ASSERT(unsupportedExt == requiredExtensions.end(),
			"Required extension not supported: {}", std::string(*unsupportedExt));

		vk::InstanceCreateInfo createInfo;
		createInfo.pApplicationInfo        = &appInfo;
		createInfo.enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size());
		createInfo.ppEnabledLayerNames     = requiredLayers.data();
		createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		m_Instance = vk::raii::Instance(m_Context, createInfo);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Instance);
	}

	std::vector<const char*> VulkanRenderAPI::GetRequiredInstanceExtensions()
	{
		uint32_t glfwCount = 0;
		const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwCount);
		std::vector<const char*> extensions(glfwExt, glfwExt + glfwCount);
		if (enableValidationLayers)
			extensions.push_back(vk::EXTDebugUtilsExtensionName);
		return extensions;
	}

	void VulkanRenderAPI::SetupDebugMessenger()
	{
		if (!enableValidationLayers)
			return;

		vk::DebugUtilsMessengerCreateInfoEXT info;
		info.messageSeverity =
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		info.messageType =
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
		info.pfnUserCallback = &DebugCallback;

		try { m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(info); }
		catch (vk::SystemError& e)
		{
			LOG(LogLevel::Error, "Debug messenger unavailable: {}", e.what());
		}
	}

	void VulkanRenderAPI::CreateMaterialDescriptorSetLayout()
	{
		// Set 1 bindings — match Mesh.slang set(1) declarations:
		//   binding 0 — albedo         (combinedImageSampler)
		//   binding 1 — normal         (combinedImageSampler)
		//   binding 2 — metallicRough  (combinedImageSampler)
		//   binding 3 — ao             (combinedImageSampler)
		//   binding 4 — emissive       (combinedImageSampler)
		//   binding 5 — MaterialParams (uniformBuffer)
		constexpr auto fragStage = vk::ShaderStageFlagBits::eFragment;
		std::array bindings = {
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, fragStage),
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, fragStage),
			vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, 1, fragStage),
			vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eCombinedImageSampler, 1, fragStage),
			vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eCombinedImageSampler, 1, fragStage),
			vk::DescriptorSetLayoutBinding(5, vk::DescriptorType::eUniformBuffer,        1, fragStage),
		};

		vk::DescriptorSetLayoutCreateInfo info;
		info.bindingCount = static_cast<uint32_t>(bindings.size());
		info.pBindings    = bindings.data();
		m_MaterialDescriptorSetLayout = vk::raii::DescriptorSetLayout(m_VulkanDevice->GetDevice(), info);

		LOG(LogLevel::Info, "VulkanRenderAPI: material descriptor set layout created (set 1, 5 textures + UBO).");
	}

	Ref<Material> VulkanRenderAPI::CreateMaterial()
	{
		return CreateRef<VulkanMaterial>();
	}

	void VulkanRenderAPI::CreateSurface(void* windowHandle)
	{
		VkSurfaceKHR raw = VK_NULL_HANDLE;
		VkResult result  = glfwCreateWindowSurface(*m_Instance, static_cast<GLFWwindow*>(windowHandle), nullptr, &raw);
		ASSERT(result == VK_SUCCESS, "Failed to create window surface.")
		m_Surface = vk::raii::SurfaceKHR(m_Instance, raw);
	}
}
