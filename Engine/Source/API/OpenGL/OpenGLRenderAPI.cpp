#include "kepch.h"
#include "OpenGLRenderAPI.h"

#include "glad/gl.h"

namespace Kairos
{
	void OpenGLRenderAPI::Init()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void OpenGLRenderAPI::SetViewport(GraphicsContext* ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRenderAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
	{

	}
}