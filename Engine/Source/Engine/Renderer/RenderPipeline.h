#pragma once

#include "Engine/Core/Base.h"

#include "entt.hpp"

namespace Engine
{
	class Framebuffer;
	class SceneRenderer;
	class ShadowPass;

	enum class RenderStage : uint8_t
	{
		ShadowPass = 0,
		GeometryPass,
		LightingPass
	};

	class RenderPipeline
	{
	public:
		explicit RenderPipeline(SceneRenderer& renderer);
		~RenderPipeline();

		void Execute(entt::registry& registry);

		Framebuffer* GetShadowMapFramebuffer() const;

	private:
		SceneRenderer&   m_Renderer;
		Scope<ShadowPass> m_ShadowPass;
	};
}
