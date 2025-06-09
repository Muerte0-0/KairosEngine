#include "kepch.h"

#include "VulkanImguiLayer.h"

#include "VulkanContext.h"
#include "Engine/Core/Application.h"
#include "VulkanUtils.h"
#include "Components/Command.h"
#include "Components/Image.h"
#include "Components/Device.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "volk.h"

IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING

namespace Kairos
{
	VulkanImGuiLayer::VulkanImGuiLayer()
	{

	}

	VulkanImGuiLayer::~VulkanImGuiLayer()
	{
		
	}

	void VulkanImGuiLayer::OnAttach()
	{
		KE_PROFILE_FUNCTION();

		// Setup Dear ImGui Context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		SetImGuiStyle();
		SetDarkThemeColors();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		VulkanContext* vctx = (VulkanContext*)app.GetWindow().GetGraphicsContext();

		ImGui_ImplGlfw_InitForVulkan(window, true);
		InitImGuiForVulkan();
	}

	void VulkanImGuiLayer::OnDetach()
	{
		KE_PROFILE_FUNCTION();

		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanImGuiLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void VulkanImGuiLayer::End()
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* current_context_backup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(current_context_backup);
		}
	}

	void VulkanImGuiLayer::InitImGuiForVulkan()
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {};
		pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingCreateInfo.viewMask = 0;
		pipelineRenderingCreateInfo.colorAttachmentCount = 1;
		pipelineRenderingCreateInfo.pColorAttachmentFormats = &vctx->GetVkContext().Swapchain.Info().ImageFormat.format;

		// Initialize ImGui For Vulkan
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = vctx->GetVkContext().Instance;
		initInfo.PhysicalDevice = vctx->GetVkContext().PhysicalDevice;
		initInfo.Device = vctx->GetVkContext().LogicalDevice;
		initInfo.Queue = vctx->GetVkContext().GraphicsQueue;
		initInfo.QueueFamily = FindQueueFamilyIndex(vctx->GetVkContext().PhysicalDevice, vctx->GetVkContext().Surface, VK_QUEUE_GRAPHICS_BIT);
		initInfo.DescriptorPool = vctx->GetVkContext().DescriptorPool;
		initInfo.MinImageCount = 2;
		initInfo.ImageCount = 2;
		initInfo.RenderPass = nullptr;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.UseDynamicRendering = true; // Using Dynamic Rendering [Vulkan 1.3 & Above]
		initInfo.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo;
		initInfo.Allocator = nullptr;

		ImGui_ImplVulkan_Init(&initInfo);
	}

	void VulkanImGuiLayer::OnImGuiRender()
	{
		
	}
}