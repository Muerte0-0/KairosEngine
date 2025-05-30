#include "kepch.h"

#include "VulkanImguiLayer.h"

#include "Engine/Core/Application.h"
#include "VulkanContext.h"
#include "Components/Command.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

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
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;          // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;         // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;              // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;            // Enable Viewports

		// Setup Dear ImGui Style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		VK_CHECK(vkCreateDescriptorPool(vctx->GetVkContext().device, &pool_info, nullptr, &m_ImguiPool));

		//this initializes the core structures of imgui
		ImGui::CreateContext();

		VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {};
		pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipeline_rendering_create_info.viewMask = 0; // No multiview support
		pipeline_rendering_create_info.colorAttachmentCount = 1;
		pipeline_rendering_create_info.pColorAttachmentFormats = &vctx->GetVkContext().m_Swapchain.GetSwapchainInfo().imageFormat.format;

		//this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vctx->GetVkContext().instance;
		init_info.PhysicalDevice = vctx->GetVkContext().physicalDevice;
		init_info.Device = vctx->GetVkContext().device;
		init_info.Queue = vctx->GetVkContext().graphicsQueue;
		init_info.DescriptorPool = m_ImguiPool;
		init_info.MinImageCount = 3;
		init_info.ImageCount = 3;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.UseDynamicRendering = true; // Use dynamic rendering for Vulkan 1.3 and above
		init_info.PipelineRenderingCreateInfo = pipeline_rendering_create_info;

		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_Init(&init_info);

		vctx->GetDeviceDeletionQueue().push_back([=](VkDevice device) 
		{
			vkDestroyDescriptorPool(device, m_ImguiPool, nullptr);
			ImGui_ImplVulkan_Shutdown();
		});
	}

	void VulkanImGuiLayer::OnDetach()
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

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

		VkCommandBuffer cb = BeginSingleTimeCommands(vctx);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb);
		EndSingleTimeCommands(vctx, cb);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* current_context_backup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(current_context_backup);
		}
	}

	void VulkanImGuiLayer::OnImGuiRender()
	{
		static bool show = true;
		ImGui::ShowDemoWindow(&show);
	}
}