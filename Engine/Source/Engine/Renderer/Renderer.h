#pragma once

#include "RenderCommand.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Framebuffer.h"

#include "GraphicsContext.h"

namespace Kairos
{
	class Renderer
	{
	public:
		static void Init();

		static void Init_Framebuffer(FramebufferSpecification spec) { m_Framebuffer = Framebuffer::Create(spec); }

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene();
		static void EndScene();

		static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		inline static RenderAPI::API GetAPI() { return RenderAPI::GetAPI(); }
		inline static Ref<Framebuffer> GetFramebuffer() { return m_Framebuffer; }
		inline static ShaderLibrary& GetShaderLibrary() { return m_ShaderLibrary; }

	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static ShaderLibrary m_ShaderLibrary;

		static Ref<Framebuffer> m_Framebuffer;

		static SceneData* m_SceneData;
	};
}