#include "kepch.h"
#include "VulkanImGuiLayer.h"

#include "imgui.h"
#include "VulkanRenderAPI.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "Engine/Core/Application.h"
#include "Engine/Renderer/Renderer.h"

#include "Engine/ImGui/ImGuiUtils.h"

namespace Engine
{
	VulkanImGuiLayer::VulkanImGuiLayer()
	{
		VulkanRenderAPI* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
        
		VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000;
		poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
		poolInfo.pPoolSizes = poolSizes;
		
		m_ImGuiPool = vk::raii::DescriptorPool(api->GetVulkanDevice()->GetDevice(), poolInfo);
	}

	void VulkanImGuiLayer::OnAttach()
	{		
		// Setup Dear ImGui Context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiUtils::SetImGuiStyle(Application::Get().GetApplicationSpecs().Theme);
        
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetHandle());
		ImGui_ImplGlfw_InitForVulkan(window, true);
        
		InitializeVulkanForImGui();
	}

	void VulkanImGuiLayer::OnDetach()
	{
		VulkanRenderAPI* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
		api->GetVulkanDevice()->WaitIdle();
		
		ImGui_ImplVulkan_Shutdown();
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
		VulkanRenderAPI* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
		
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *api->GetActiveCommandBuffer());
		
		ImGuiIO& io = ImGui::GetIO();
        
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void VulkanImGuiLayer::OnImGuiRender()
	{
		//ImGui::ShowDemoWindow();
	}

	void VulkanImGuiLayer::InitializeVulkanForImGui() const
	{
		VulkanRenderAPI* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
		
		// This initializes ImGui for Vulkan
		ImGui_ImplVulkan_InitInfo iniInfo = {};
		iniInfo.Instance = *api->GetInstance();
		iniInfo.PhysicalDevice = *api->GetVulkanDevice()->GetPhysicalDevice();
		iniInfo.Device = *api->GetVulkanDevice()->GetDevice();
		iniInfo.Queue = *api->GetVulkanDevice()->GetGraphicsQueue();
		iniInfo.DescriptorPool = *m_ImGuiPool;
		iniInfo.MinImageCount = 3;
		iniInfo.ImageCount = 3;
		iniInfo.UseDynamicRendering = true;

		//dynamic rendering parameters for imgui to use
		iniInfo.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		iniInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		vk::Format format = api->GetVulkanSwapchain()->GetSwapChainImageFormat();
		VkFormat vkFormat = static_cast<VkFormat>(format);
		iniInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &vkFormat;
		iniInfo.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		iniInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&iniInfo);
	}
}
