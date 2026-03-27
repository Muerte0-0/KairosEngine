#include "EditorLayer.h"

EditorLayer::EditorLayer()
{
}

void EditorLayer::OnAttach()
{
    Layer::OnAttach();
}

void EditorLayer::OnDetach()
{
    Layer::OnDetach();
}

void EditorLayer::OnUpdate(float DeltaTime)
{
    Layer::OnUpdate(DeltaTime);
}

void EditorLayer::OnFixedUpdate(float DeltaTime)
{
    Layer::OnFixedUpdate(DeltaTime);
}

void EditorLayer::OnRender()
{
    Layer::OnRender();
}

void EditorLayer::OnImGuiRender()
{
    static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags dockspaceWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    dockspaceWindowFlags |= ImGuiWindowFlags_NoTitleBar;
    dockspaceWindowFlags |= ImGuiWindowFlags_NoCollapse;
    dockspaceWindowFlags |= ImGuiWindowFlags_NoResize;
    dockspaceWindowFlags |= ImGuiWindowFlags_NoMove;
    dockspaceWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    dockspaceWindowFlags |= ImGuiWindowFlags_NoNavFocus;
    dockspaceWindowFlags |= ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Editor Dockspace", nullptr, dockspaceWindowFlags);

    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("Editor Dockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);
    }

    DrawMenuBar();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    DrawViewport();

    ImGui::PopStyleVar();

    ImGui::Begin("Scene Hierarchy");
    ImGui::End();

    ImGui::Begin("Content Browser");
    ImGui::End();

    if (m_ShowConsole)
        DrawConsole();

    ImGui::Begin("Details");
    ImGui::End();

    DrawImGuiDebug();

    ImGui::End();
}

void EditorLayer::OnEvent(Engine::Event& event)
{
    Layer::OnEvent(event);
}

void EditorLayer::DrawMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Project", "Ctrl+N"))
				{
					// Handle new project	
				}

				if (ImGui::MenuItem("Open Project", "Ctrl+O"))
				{
					// Handle open project
				}

				if (ImGui::MenuItem("Save Project", "Ctrl+S"))
				{
					// Handle save project
				}

				if (ImGui::MenuItem("Save Project As", "Ctrl+Shift+S"))
				{
					// Handle save project as
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("New Script"))
				{
					// Handle Creating a new script
				}

				if (ImGui::MenuItem("Asset Browser"))
				{
					// Handle asset browser
				}

				if (ImGui::MenuItem("Console", nullptr, &m_ShowConsole))
				{
					LOG(Engine::LogLevel::Info, "Console {}", m_ShowConsole ? "Shown" : "Hidden");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("About"))
			{
				if (ImGui::MenuItem("About Kairos Engine"))
				{
					// Handle about
				}

				if (ImGui::MenuItem("Documentation"))
				{
					// Handle documentation
				}

				if (ImGui::MenuItem("GitHub Repository"))
				{
					string url = "https://github.com/Muerte0-0/KairosEngine";
					
#ifdef PLATFORM_WINDOWS
					// Use ShellExecute on Windows
					system(("start " + url).c_str());
#else
#if PLATFORM_LINUX
					system(("xdg-open " + url).c_str());
#endif
#endif
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

void EditorLayer::DrawViewport()
	{
		ImGui::Begin("Viewport");
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);

		//ITextureView* textureID = Renderer::GetFramebuffer()->GetImGuiTextureID();
		//if (textureID != nullptr)
		//{
		//	ImTextureID imguiTextureID = reinterpret_cast<ImTextureID>(textureID);
		//	ImGui::Image(imguiTextureID, viewportPanelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		//}
		//else
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Implement Yet! :)");
		}

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && m_ViewportHovered)
			m_ViewportRightClicked = true;
		else
			m_ViewportRightClicked = false;

		//Input::SetCursorLockMode(m_ViewportRightClicked ? CursorMode::Locked : CursorMode::Normal);

		ImGui::End();
	}

void EditorLayer::DrawImGuiDebug()
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui::Begin("Debug Info");

		ImGui::Text("ConfigFlags:");
		ImGui::Text("  ViewportsEnable: %d", (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0);
		ImGui::Text("  DockingEnable: %d", (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0);

		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		ImGui::Text("Viewports: %d", platform_io.Viewports.Size);

		for (int i = 0; i < platform_io.Viewports.Size; i++)
		{
			ImGuiViewport* vp = platform_io.Viewports[i];
			ImGui::Text("  Viewport %d: Pos(%.0f, %.0f) Size(%.0f, %.0f) DrawData: %s",
						i, vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y,
						vp->DrawData ? "Yes" : "No");
		}
		ImGui::End();
	}

void EditorLayer::DrawConsole()
{
	
}
