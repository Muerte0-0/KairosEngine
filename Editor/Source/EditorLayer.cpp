#include "EditorLayer.h"

namespace Kairos
{
	EditorLayer::EditorLayer() : Layer("Kairos Editor")
	{
		
	}

	void EditorLayer::OnAttach()
	{
		KE_PROFILE_FUNCTION();

		//m_VertexArray.reset(VertexArray::Create());
		//
		//float vertices[3 * 7] = {
		//	-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
		//	 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
		//	 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		//};
		//
		//Ref<VertexBuffer> vertexBuffer;
		//vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		//BufferLayout layout = {
		//	{ ShaderDataType::Float3, "a_Position" },
		//	{ ShaderDataType::Float4, "a_Color" }
		//};
		//vertexBuffer->SetLayout(layout);
		//m_VertexArray->AddVertexBuffer(vertexBuffer);
		//
		//uint32_t indices[3] = { 0, 1, 2 };
		//Ref<IndexBuffer> indexBuffer;
		//indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		//m_VertexArray->SetIndexBuffer(indexBuffer);

		m_TriangleShader = Shader::Create("Assets/Shaders/Triangle/Triangle.glsl");

		FramebufferSpecification spec;
		spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		spec.Width = 1280;
		spec.Height = 720;
		//m_Framebuffer = Framebuffer::Create(spec);
	}

	void EditorLayer::OnDetach()
	{
		KE_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Timestep delta)
	{
		KE_PROFILE_FUNCTION();

		//if (FramebufferSpecification spec = m_Framebuffer->GetSpecification(); 
		//m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // Zero sized Framebuffer is Invalid
		//(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		//{
		//	m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		//}
		
		KE_PROFILE_SCOPE("Renderer Prep");
		//m_Framebuffer->Bind();
		RenderCommand::Clear();

		//m_Framebuffer->ClearAttachment(1, -1);

		Renderer::Submit(m_TriangleShader, m_VertexArray, NULL);

		//m_Framebuffer->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{
		KE_PROFILE_FUNCTION();

		return;

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

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
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);
			//uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
			//ImGui::Image(textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();

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