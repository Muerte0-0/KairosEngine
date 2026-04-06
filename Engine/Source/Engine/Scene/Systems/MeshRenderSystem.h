#pragma once

#include "entt.hpp"

namespace Engine
{
	class SceneRenderer;

	namespace MeshRenderSystem
	{
		void Render(entt::registry& registry, SceneRenderer& renderer);
	}
}
