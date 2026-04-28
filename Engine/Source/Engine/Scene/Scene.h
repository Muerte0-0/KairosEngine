#pragma once

#include "Engine/Core/UUID.h"
#include "SceneGraph.h"

#include "entt.hpp"
#include <functional>

namespace Kairos
{
	class SceneHierarchyPanel;
}

namespace Engine
{
	class Entity;
	class SceneSerializer;
	class SceneRenderer;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = "Empty Entity");
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Empty Entity");
		void DestroyEntity(Entity entity);

		void OnUpdate(float deltaTime);

		const std::string& GetName() const { return m_Name; }
		void               SetName(const std::string& name) { m_Name = name; }

		void OnRender(SceneRenderer& renderer);

		void EachEntity(const std::function<void(Entity)>& fn);

		// Primary camera queries. Only one CameraComponent can have Primary == true.
		Entity GetPrimaryCamera();
		// Sets entity as primary; clears Primary flag on all others.
		void   SetPrimaryCamera(Entity entity);

		SceneGraph& GetSceneGraph() { return m_SceneGraph; }
		entt::registry& GetRegistry() { return m_Registry; }

		// Returns the UUID of entity's parent, or 0 if root.
		uint64_t GetParentUUID(EntityID id) const;

	private:
		void PropagateTransforms();

		entt::registry m_Registry;
		SceneGraph     m_SceneGraph;
		std::string    m_Name = "Untitled";
		
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		friend class Entity;
		friend class SceneSerializer;
		friend class Kairos::SceneHierarchyPanel;
	};
}
