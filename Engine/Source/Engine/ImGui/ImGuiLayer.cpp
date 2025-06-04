#include "kepch.h"
#include "ImGuiLayer.h"

#include "Engine/Renderer/RenderAPI.h"

#include "imgui.h"

#include "API/OpenGL/OpenGLImguiLayer.h"
#include "API/Vulkan/VulkanImGuiLayer.h"

namespace Kairos
{
    ImGuiLayer* Kairos::ImGuiLayer::Create()
    {
		switch (RenderAPI::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> ImGui Not Implemented!");
			case RenderAPI::API::OpenGL:			return new OpenGLImGuiLayer();
			case RenderAPI::API::Vulkan:			return new VulkanImGuiLayer();
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> ImGui Not Implemented!");
			#endif // KE_PLATFORM_WINDOWS
		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
    }

	void ImGuiLayer::SetImGuiStyle()
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;			// Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;			// Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;				// Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;				// Enable Viewports
		io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;						// Enable SRGB for ImGui

		// Setup Dear ImGui Style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		float fontSize = 18.0f;
		io.FontDefault = io.Fonts->AddFontFromFileTTF("D:/Dev/KairosEngine/Editor/Assets/Fonts/OpenSans/OpenSans-Regular.ttf", fontSize);
		io.Fonts->AddFontFromFileTTF("D:/Dev/KairosEngine/Editor/Assets/Fonts/OpenSans/OpenSans-Bold.ttf", fontSize);

		ImGuiStyle& style = ImGui::GetStyle();
		style.FramePadding = ImVec2(10.0f, 6.0f);
		style.ItemSpacing = ImVec2(6.0f, 6.0f);
		style.WindowRounding = 8.0f;
		style.ChildRounding = 8.0f;
		style.FrameRounding = 6.0f;
		style.PopupRounding = 6.0f;
		style.ScrollbarRounding = 6.0f;
		style.GrabRounding = 6.0f;
		style.TabRounding = 6.0f;
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	}

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabSelected] = ImVec4{ 0.32f, 0.3205f, 0.321f, 1.0f };
		colors[ImGuiCol_TabSelectedOverline] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}
}
