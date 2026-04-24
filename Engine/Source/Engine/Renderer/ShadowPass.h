#pragma once

#include "RenderPass.h"
#include "RHI/Framebuffer.h"
#include "RHI/GraphicsPipeline.h"
#include "RHI/Resources/Mesh.h"
#include "SceneRenderer.h"

#include "Engine/Scene/Systems/ShadowRenderSystem.h"

#include "entt.hpp"

namespace Engine
{
	class ShadowPass
	{
	public:
		void Init();
		void Begin();
		[[nodiscard]] ShadowSceneData Render(entt::registry& registry);
		void End();

		void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& modelTransform, const glm::mat4& lightViewProjection);

		Framebuffer* GetFramebuffer() const { return m_Framebuffer.get(); }

	private:
		void CreateFramebuffer();
		void CreatePipeline(const BufferLayout& vertexLayout);

		static constexpr uint32_t SHADOW_MAP_SIZE = 2048;

		RenderPass              m_RenderPass;
		Scope<Framebuffer>      m_Framebuffer;
		Scope<GraphicsPipeline> m_Pipeline;
		ShadowRenderSystem::LightRenderData m_LightData;
	};
}
