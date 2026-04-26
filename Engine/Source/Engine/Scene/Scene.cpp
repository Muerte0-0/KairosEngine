#include "kepch.h"
#include "Scene.h"

#include "Components.h"
#include "Entity.h"
#include "SceneGraph.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Systems/MeshRenderSystem.h"

namespace Engine
{
	Scene::Scene()
	{
		
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name;

		m_SceneGraph.AddEntity(static_cast<entt::entity>(entity));
		
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_SceneGraph.RemoveEntity(static_cast<entt::entity>(entity));
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdate(float deltaTime)
	{
		PropagateTransforms();
	}

	uint64_t Scene::GetParentUUID(EntityID id) const
	{
		const SceneNode* node = m_SceneGraph.GetNode(id);
		if (!node || node->Parent == INVALID_ENTITY) return 0;

		auto parentEntt = static_cast<entt::entity>(node->Parent);
		if (!m_Registry.valid(parentEntt)) return 0;

		const auto* parentID = m_Registry.try_get<IDComponent>(parentEntt);
		return parentID ? static_cast<uint64_t>(parentID->ID) : 0;
	}

	void Scene::PropagateTransforms()
	{
		// Recursive lambda — walks SceneGraph depth-first from roots
		auto propagate = [&](auto& self, EntityID id, const glm::mat4& parentWorld) -> void
		{
			SceneNode* node = m_SceneGraph.GetNode(id);
			if (!node) return;

			auto enttID = static_cast<entt::entity>(id);
			if (!m_Registry.valid(enttID)) return;

			auto* tc = m_Registry.try_get<TransformComponent>(enttID);
			if (!tc) return;

			tc->WorldTransform = parentWorld * tc->GetTransform();

			for (EntityID child : node->Children)
				self(self, child, tc->WorldTransform);
		};

		for (EntityID root : m_SceneGraph.GetRootNodes())
			propagate(propagate, root, glm::mat4(1.0f));
	}

	void Scene::OnRender(SceneRenderer& renderer)
	{
		renderer.Render(m_Registry);
	}

	void Scene::EachEntity(const std::function<void(Entity)>& fn)
	{
		for (auto entity : std::views::reverse(m_Registry.view<entt::entity>()))
			fn(Entity{ entity, this });
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& component)
	{
		
	}
}
