#include "EditorLayer.h"

namespace Kairos
{
	EditorLayer::EditorLayer() : Layer("Kairos Editor")
	{
		
	}

	void EditorLayer::OnAttach()
	{
		//m_TriangleShader = Shader::Create("Assets/Shaders/Triangle/Triangle.glsl");
	}

	void EditorLayer::OnDetach()
	{
		
	}

	void EditorLayer::OnUpdate(Timestep delta)
	{
		RenderCommand::Clear();
		//Renderer::Submit(m_TriangleShader, m_VertexArray, NULL);
	}

	void EditorLayer::OnImGuiRender()
	{

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::Begin("Editor Dockspace", nullptr, window_flags);

		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("Editor Dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

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

				if (ImGui::MenuItem("Console"))
				{
					// Handle console
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
					// Handle GitHub repository
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin("Viewport");
		ImGui::End();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		
	}

}