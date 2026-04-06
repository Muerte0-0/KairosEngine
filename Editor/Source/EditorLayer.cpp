#include "EditorLayer.h"

#include "Engine/Renderer/RHI/Buffer.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"

#include <iterator>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
	Ref<Mesh> CreateDefaultCubeMesh()
	{
		struct VertexData
		{
			glm::vec3 Position;
			glm::vec3 Color;
		};

		constexpr VertexData vertices[] = {
			{ { -0.5f, -0.5f,  0.5f }, { 0.95f, 0.25f, 0.25f } },
			{ {  0.5f, -0.5f,  0.5f }, { 0.95f, 0.25f, 0.25f } },
			{ {  0.5f,  0.5f,  0.5f }, { 0.95f, 0.25f, 0.25f } },
			{ { -0.5f,  0.5f,  0.5f }, { 0.95f, 0.25f, 0.25f } },

			{ { -0.5f, -0.5f, -0.5f }, { 0.25f, 0.95f, 0.35f } },
			{ {  0.5f, -0.5f, -0.5f }, { 0.25f, 0.95f, 0.35f } },
			{ {  0.5f,  0.5f, -0.5f }, { 0.25f, 0.95f, 0.35f } },
			{ { -0.5f,  0.5f, -0.5f }, { 0.25f, 0.95f, 0.35f } },

			{ { -0.5f, -0.5f, -0.5f }, { 0.25f, 0.55f, 0.95f } },
			{ { -0.5f, -0.5f,  0.5f }, { 0.25f, 0.55f, 0.95f } },
			{ { -0.5f,  0.5f,  0.5f }, { 0.25f, 0.55f, 0.95f } },
			{ { -0.5f,  0.5f, -0.5f }, { 0.25f, 0.55f, 0.95f } },

			{ {  0.5f, -0.5f, -0.5f }, { 0.95f, 0.85f, 0.25f } },
			{ {  0.5f, -0.5f,  0.5f }, { 0.95f, 0.85f, 0.25f } },
			{ {  0.5f,  0.5f,  0.5f }, { 0.95f, 0.85f, 0.25f } },
			{ {  0.5f,  0.5f, -0.5f }, { 0.95f, 0.85f, 0.25f } },

			{ { -0.5f,  0.5f,  0.5f }, { 0.85f, 0.25f, 0.95f } },
			{ {  0.5f,  0.5f,  0.5f }, { 0.85f, 0.25f, 0.95f } },
			{ {  0.5f,  0.5f, -0.5f }, { 0.85f, 0.25f, 0.95f } },
			{ { -0.5f,  0.5f, -0.5f }, { 0.85f, 0.25f, 0.95f } },

			{ { -0.5f, -0.5f,  0.5f }, { 0.25f, 0.95f, 0.95f } },
			{ {  0.5f, -0.5f,  0.5f }, { 0.25f, 0.95f, 0.95f } },
			{ {  0.5f, -0.5f, -0.5f }, { 0.25f, 0.95f, 0.95f } },
			{ { -0.5f, -0.5f, -0.5f }, { 0.25f, 0.95f, 0.95f } },
		};

		constexpr uint32_t indices[] = {
			0, 1, 2, 2, 3, 0,
			4, 6, 5, 6, 4, 7,
			8, 9, 10, 10, 11, 8,
			12, 14, 13, 14, 12, 15,
			16, 17, 18, 18, 19, 16,
			20, 22, 21, 22, 20, 23
		};

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "inPosition" },
			{ ShaderDataType::Float3, "inColor" }
		});

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, static_cast<uint32_t>(std::size(indices)));
		return CreateRef<Mesh>(vertexBuffer, indexBuffer);
	}
}

void EditorLayer::OnAttach()
{
    Layer::OnAttach();

    m_ActiveScene = CreateRef<Scene>();
    m_SceneRenderer = CreateScope<SceneRenderer>();

	m_CubeEntity = m_ActiveScene->CreateEntity("Viewport Cube");
	m_CubeEntity.AddComponent<MeshComponent>().Mesh = CreateDefaultCubeMesh();
	m_CubeEntity.GetComponent<TransformComponent>().Transform =
		glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, -2.5f));
	
	Entity cube2 = m_ActiveScene->CreateEntity("Viewport Cube 2");
	cube2.AddComponent<MeshComponent>().Mesh = CreateDefaultCubeMesh();
	cube2.GetComponent<TransformComponent>().Transform =
		glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, -2.5f));
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
	
	m_CubeRotation += DeltaTime * glm::radians(45.0f);

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, -2.5f));
	transform = glm::rotate(transform, m_CubeRotation, glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::rotate(transform, m_CubeRotation * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));

	m_CubeEntity.GetComponent<TransformComponent>().Transform = transform;
}

void EditorLayer::OnRender()
{
    Layer::OnRender();

    if (!m_SceneRenderer)
        return;

    const float width = (std::max)(m_ViewportSize.x, 1.0f);
    const float height = (std::max)(m_ViewportSize.y, 1.0f);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
    projection[1][1] *= -1.0f;

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 2.5f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    Camera viewportCamera(view, projection);
    m_SceneRenderer->BeginScene(viewportCamera);
    m_ActiveScene->OnRender(*m_SceneRenderer);
    m_SceneRenderer->EndScene();
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
        ImGuiID dockspaceID = ImGui::GetID("Editor Dockspace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
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
            }

            if (ImGui::MenuItem("Open Project", "Ctrl+O"))
            {
            }

            if (ImGui::MenuItem("Save Project", "Ctrl+S"))
            {
            }

            if (ImGui::MenuItem("Save Project As", "Ctrl+Shift+S"))
            {
            }

            if (ImGui::MenuItem("Exit"))
            {
                Engine::Application::Get().Stop();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::MenuItem("New Script"))
            {
            }

            if (ImGui::MenuItem("Asset Browser"))
            {
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
            }

            if (ImGui::MenuItem("Documentation"))
            {
            }

            if (ImGui::MenuItem("GitHub Repository"))
            {
                string url = "https://github.com/Muerte0-0/KairosEngine";

#ifdef PLATFORM_WINDOWS
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

    Engine::Framebuffer* framebuffer = m_SceneRenderer ? m_SceneRenderer->GetFramebuffer() : nullptr;
    if (framebuffer != nullptr && viewportPanelSize.x > 0.0f && viewportPanelSize.y > 0.0f)
    {
        m_SceneRenderer->Resize(
            static_cast<uint32_t>(viewportPanelSize.x),
            static_cast<uint32_t>(viewportPanelSize.y));

        if (void* textureID = framebuffer->GetImGuiTextureID())
        {
            ImGui::Image(textureID, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Implemented Yet! :)");
    }

    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && m_ViewportHovered)
        m_ViewportRightClicked = true;
    else
        m_ViewportRightClicked = false;

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
