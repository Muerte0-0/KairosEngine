#pragma once

#include "Engine/Core/Base.h"

#include "entt.hpp"
#include <glm/glm.hpp>

namespace Engine
{
	class ShadowPass;

	namespace ShadowRenderSystem
	{
		struct LightRenderData
		{
			glm::mat4 LightView{ 1.0f };
			glm::mat4 LightProjection{ 1.0f };
			glm::mat4 LightViewProjection{ 1.0f };
			glm::vec3 Direction{ 0.0f, -1.0f, 0.0f };
			float     Bias = 0.0015f;
			float     TexelSize = 0.0f;
			int       LightEntityID = -1;
			bool      Valid = false;
		};

		[[nodiscard]] LightRenderData ExtractDirectionalLight(entt::registry& registry, uint32_t shadowMapSize);
		void Render(entt::registry& registry, ShadowPass& shadowPass, const LightRenderData& lightData);
	}
}
