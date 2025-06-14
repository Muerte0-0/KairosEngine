#include "EditorLayer.h"

namespace Kairos
{
	EditorLayer::EditorLayer() : Layer("Kairos Editor"), m_PerspectiveCamera(45.0f, 1280, 720)
	{
		
	}

	void EditorLayer::OnAttach()
	{
		KE_PROFILE_FUNCTION();

		m_VertexArray.reset(VertexArray::Create());

		
		std::array<Vertex, 24> vertexData;

		// Front
		vertexData[0].Position = glm::vec3(-0.5f, -0.5f, 0.5f);
		vertexData[0].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertexData[1].Position = glm::vec3(-0.5f, 0.5f, 0.5f);
		vertexData[1].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertexData[2].Position = glm::vec3(0.5f, 0.5f, 0.5f);
		vertexData[2].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertexData[3].Position = glm::vec3(0.5f, -0.5f, 0.5f);
		vertexData[3].Normal = glm::vec3(0.0f, 0.0f, 1.0f);

		// Right
		vertexData[4].Position = glm::vec3(0.5f, -0.5f, 0.5f);
		vertexData[4].Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertexData[5].Position = glm::vec3(0.5f, 0.5f, 0.5f);
		vertexData[5].Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertexData[6].Position = glm::vec3(0.5f, 0.5f, -0.5f);
		vertexData[6].Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertexData[7].Position = glm::vec3(0.5f, -0.5f, -0.5f);
		vertexData[7].Normal = glm::vec3(1.0f, 0.0f, 0.0f);

		// Back
		vertexData[8].Position = glm::vec3(0.5f, -0.5f, -0.5f);
		vertexData[8].Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertexData[9].Position = glm::vec3(0.5f, 0.5f, -0.5f);
		vertexData[9].Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertexData[10].Position = glm::vec3(-0.5f, 0.5f, -0.5f);
		vertexData[10].Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertexData[11].Position = glm::vec3(-0.5f, -0.5f, -0.5f);
		vertexData[11].Normal = glm::vec3(0.0f, 0.0f, -1.0f);

		// Left
		vertexData[12].Position = glm::vec3(-0.5f, -0.5f, -0.5f);
		vertexData[12].Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertexData[13].Position = glm::vec3(-0.5f, 0.5f, -0.5f);
		vertexData[13].Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertexData[14].Position = glm::vec3(-0.5f, 0.5f, 0.5f);
		vertexData[14].Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertexData[15].Position = glm::vec3(-0.5f, -0.5f, 0.5f);
		vertexData[15].Normal = glm::vec3(-1.0f, 0.0f, 0.0f);

		// Top
		vertexData[16].Position = glm::vec3(-0.5f, 0.5f, 0.5f);
		vertexData[16].Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertexData[17].Position = glm::vec3(-0.5f, 0.5f, -0.5f);
		vertexData[17].Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertexData[18].Position = glm::vec3(0.5f, 0.5f, -0.5f);
		vertexData[18].Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertexData[19].Position = glm::vec3(0.5f, 0.5f, 0.5f);
		vertexData[19].Normal = glm::vec3(0.0f, 1.0f, 0.0f);

		// Bottom
		vertexData[20].Position = glm::vec3(-0.5f, -0.5f, -0.5f);
		vertexData[20].Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertexData[21].Position = glm::vec3(-0.5f, -0.5f, 0.5f);
		vertexData[21].Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertexData[22].Position = glm::vec3(0.5f, -0.5f, 0.5f);
		vertexData[22].Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertexData[23].Position = glm::vec3(0.5f, -0.5f, -0.5f);
		vertexData[23].Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		

		Ref<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create((float*)vertexData.data(), vertexData.size() * sizeof(Vertex)));
		BufferLayout layout =
		{
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		
		std::array<uint32_t, 36> indices;

		uint32_t offset = 0;

		for (int i = 0; i < 36; i += 6)
		{
			indices[i + 0] = 0 + offset;
			indices[i + 1] = 1 + offset;
			indices[i + 2] = 2 + offset;
			indices[i + 3] = 2 + offset;
			indices[i + 4] = 3 + offset;
			indices[i + 5] = 0 + offset;

			offset += 4;
		}

		Ref<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices.data(), indices.size()));
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

	void EditorLayer::OnUpdate(Timestep deltaTime)
	{
		KE_PROFILE_FUNCTION();

		if (FramebufferSpecification spec = Renderer::GetFramebuffer()->GetSpecification();
		m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // Zero sized Framebuffer is Invalid
		(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			Renderer::GetFramebuffer()->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_PerspectiveCamera.ViewportResized(m_ViewportSize.x, m_ViewportSize.y);
		}

		m_PerspectiveCamera.OnUpdate(deltaTime);

		Renderer::BeginScene(m_PerspectiveCamera);

		KE_PROFILE_SCOPE("Renderer Prep");
		Renderer::GetFramebuffer()->Bind();

		RenderCommand::Clear();
		Renderer::GetFramebuffer()->ClearAttachment(1, -1);

		Ref<Shader> triangleShader = Renderer::GetShaderLibrary().Get("Triangle");
		Renderer::Submit(triangleShader, m_VertexArray, NULL);

		Renderer::GetFramebuffer()->Unbind();

		Renderer::EndScene();
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