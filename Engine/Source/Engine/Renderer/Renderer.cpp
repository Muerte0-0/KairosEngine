#include "kepch.h"
#include "Renderer.h"

namespace Kairos
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;
	ShaderLibrary Renderer::m_ShaderLibrary;
	std::shared_ptr<Framebuffer> Renderer::m_Framebuffer;

	void Renderer::Init()
	{
		RenderCommand::Init();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene()
	{
		
	}

	void Renderer::EndScene()
	{
		
	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();

		RenderCommand::DrawIndexed(vertexArray);
	}
}