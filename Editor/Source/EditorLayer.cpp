#include "EditorLayer.h"


namespace Kairos
{
	EditorLayer::EditorLayer() : Layer("Kairos Editor")
	{
		
	}

	void EditorLayer::OnAttach()
	{
		KE_PROFILE_FUNCTION();

		m_VertexArray.reset(VertexArray::Create());
		
		float vertices[3 * 8] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		};
		
		Ref<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		BufferLayout layout = {
			{ ShaderDataType::Float4, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		
		uint32_t indices[3] = { 0, 1, 2 };
		Ref<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		Renderer::GetShaderLibrary().Load("D:/Dev/KairosEngine/Editor/Assets/Shaders/Triangle/Triangle.glsl");

		FramebufferSpecification spec;
		spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		spec.Width = 1280;
		spec.Height = 720;
		Renderer::Init_Framebuffer(spec);
	}

	void EditorLayer::OnDetach()
	{
		KE_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Timestep delta)
	{
		KE_PROFILE_FUNCTION();

		if (FramebufferSpecification spec = Renderer::GetFramebuffer()->GetSpecification();
		m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // Zero sized Framebuffer is Invalid
		(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			Renderer::GetFramebuffer()->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}
		
		KE_PROFILE_SCOPE("Renderer Prep");
		Renderer::GetFramebuffer()->Bind();

		RenderCommand::Clear();
		Renderer::GetFramebuffer()->ClearAttachment(1, -1);

		Ref<Shader> triangleShader = Renderer::GetShaderLibrary().Get("Triangle");

		Renderer::Submit(triangleShader, m_VertexArray, NULL);

		Renderer::GetFramebuffer()->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{
		KE_PROFILE_FUNCTION();

		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiWindowFlags dockspaceWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		dockspaceWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

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

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Viewport");
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);
			ImTextureID textureID = Renderer::GetFramebuffer()->GetColorAttachmentRendererID();
			ImGui::Image(textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();

		ImGui::PopStyleVar();

		ImGui::Begin("Scene Hierarchy");
		ImGui::End();
		
		ImGui::Begin("Content Browser");
		ImGui::End();
		
		ImGui::Begin("Console");
		ImGui::End();
		
		ImGui::Begin("Details");
		ImGui::End();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		
	}
}