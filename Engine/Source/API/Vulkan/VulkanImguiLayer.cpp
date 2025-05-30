#include "kepch.h"

#include "VulkanImguiLayer.h"

#include "Engine/Core/Application.h"
#include "VulkanContext.h"
#include "Components/Command.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "volk.h"

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
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		ImGui_ImplGlfw_InitForVulkan(window, true);

		VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {};
		pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipeline_rendering_create_info.colorAttachmentCount = 1;
		pipeline_rendering_create_info.pColorAttachmentFormats = &vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().imageFormat.format;

		// Initialize ImGui For Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vctx->GetVkContext().instance;
		init_info.PhysicalDevice = vctx->GetVkContext().physicalDevice;
		init_info.Device = vctx->GetVkContext().device;
		init_info.Queue = vctx->GetVkContext().graphicsQueue;
		init_info.DescriptorPool = vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().descriptorPool;
		init_info.MinImageCount = 2;
		init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.UseDynamicRendering = true; // Use Dynamic Rendering [Vulkan 1.3 & Above]
		init_info.PipelineRenderingCreateInfo = pipeline_rendering_create_info;

		ImGui_ImplVulkan_Init(&init_info);
	}

	void VulkanImGuiLayer::OnDetach()
	{
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
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		DrawImGui();

		vkDeviceWaitIdle(vctx->GetVkContext().device);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* current_context_backup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(current_context_backup);
		}
	}

	void VulkanImGuiLayer::DrawImGui()
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkCommandBuffer cb = BeginSingleTimeCommands(vctx);

		 VkRenderingAttachmentInfo colorAttachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().frames[vctx->GetVkContext().currentFrame].ImageView,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {.color = { 0.1f, 0.1f, 0.1f, 1.0f } }
        };

        VkRenderingInfo renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {
                .offset = { 0, 0 },
                .extent = vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().extent
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment,
        };

		vkCmdBeginRenderingKHR(cb, &renderingInfo);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb);

		vkCmdEndRenderingKHR(cb);

		EndSingleTimeCommands(vctx, cb);
	}

	void VulkanImGuiLayer::OnImGuiRender()
	{
		
	}
}