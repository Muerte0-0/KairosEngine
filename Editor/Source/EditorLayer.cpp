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
		RenderCommand::SetClearColor({ 0.1f,0.1f ,0.1f ,1.0f });
		RenderCommand::Clear();
		
		//Renderer::Submit(m_TriangleShader, m_VertexArray, NULL);
	}

	void EditorLayer::OnImGuiRender()
	{

	}

	void EditorLayer::OnEvent(Event& event)
	{
		
	}

}