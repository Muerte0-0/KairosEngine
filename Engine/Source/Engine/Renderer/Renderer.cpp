#include "kepch.h"
#include "Renderer.h"

namespace Kairos
{
	SceneData* Renderer::m_SceneData = new SceneData;
	ShaderLibrary Renderer::m_ShaderLibrary;
	Ref<Framebuffer> Renderer::m_Framebuffer;

	void Renderer::Init()
	{
		RenderCommand::Init();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		m_SceneData->CameraPosition = camera.GetPosition();
	}

	void Renderer::EndScene()
	{
		
	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();

		shader->UploadSceneData("SceneData", *m_SceneData);

		RenderCommand::DrawIndexed(vertexArray);
	}
}