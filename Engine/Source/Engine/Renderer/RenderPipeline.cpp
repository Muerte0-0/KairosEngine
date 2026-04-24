#include "kepch.h"
#include "RenderPipeline.h"

#include "SceneRenderer.h"
#include "ShadowPass.h"

namespace Engine
{
	RenderPipeline::RenderPipeline(SceneRenderer& renderer)
		: m_Renderer(renderer)
	{
		m_ShadowPass = CreateScope<ShadowPass>();
		m_ShadowPass->Init();
	}

	RenderPipeline::~RenderPipeline() = default;

	void RenderPipeline::Execute(entt::registry& registry)
	{
		m_ShadowPass->Begin();
		m_Renderer.SetShadowData(m_ShadowPass->Render(registry));
		m_ShadowPass->End();
		
		m_Renderer.ExecuteGeometryPass(registry);

	}

	Framebuffer* RenderPipeline::GetShadowMapFramebuffer() const
	{
		return m_ShadowPass ? m_ShadowPass->GetFramebuffer() : nullptr;
	}
}
