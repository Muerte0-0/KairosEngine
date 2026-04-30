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

	void Scene::DestroyEntityHierarchy(Entity entity)
	{
		EntityID id = static_cast<entt::entity>(entity);
		SceneNode* node = m_SceneGraph.GetNode(id);

		// Copy children list — RemoveEntity will mutate node's Children
		std::vector<EntityID> children;
		if (node)
			children = node->Children;

		for (EntityID childID : children)
		{
			if (m_Registry.valid(static_cast<entt::entity>(childID)))
				DestroyEntityHierarchy(Entity{ static_cast<entt::entity>(childID), this });
		}

		m_SceneGraph.RemoveEntity(id);
		m_Registry.destroy(static_cast<entt::entity>(entity));
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

	Entity Scene::GetPrimaryCamera()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			if (view.get<CameraComponent>(entity).Primary)
				return Entity{ entity, this };
		}
		return Entity{}; // null entity
	}

	void Scene::SetPrimaryCamera(Entity entity)
	{
		// Clear all primaries first.
		auto view = m_Registry.view<CameraComponent>();
		for (auto e : view)
			view.get<CameraComponent>(e).Primary = false;

		// Set the requested one.
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Primary = true;
	}

	Ref<Scene> Scene::Clone() const
	{
		auto clone = CreateRef<Scene>();
		clone->m_Name = m_Name;

		// Map old entity → new entity for SceneGraph parent/child reconstruction.
		std::unordered_map<entt::entity, entt::entity> entityMap;

		// Pass 1: create entities preserving UUIDs.
		for (auto src : std::views::reverse(m_Registry.view<entt::entity>()))
		{
			const auto* id  = m_Registry.try_get<IDComponent>(src);
			const auto* tag = m_Registry.try_get<TagComponent>(src);

			std::string name = tag ? tag->Tag : "Entity";
			UUID uuid        = id  ? id->ID   : UUID();

			Entity dst = clone->CreateEntityWithUUID(uuid, name);
			entityMap[src] = static_cast<entt::entity>(dst);
		}

		// Pass 2: copy all components (except IDComponent / TagComponent already set).
		for (auto [src, dst] : entityMap)
		{
			if (const auto* tc = m_Registry.try_get<TransformComponent>(src))
			{
				auto& c       = clone->m_Registry.get_or_emplace<TransformComponent>(dst);
				c.Translation = tc->Translation;
				c.Rotation    = tc->Rotation;
				c.Scale       = tc->Scale;
				c.WorldTransform = tc->WorldTransform;
			}
			if (const auto* mc = m_Registry.try_get<MeshComponent>(src))
			{
				auto& c            = clone->m_Registry.emplace_or_replace<MeshComponent>(dst);
				c.MeshAssetHandle  = mc->MeshAssetHandle;
				c.MaterialAssetHandle = mc->MaterialAssetHandle;
				c.PrimitiveKey     = mc->PrimitiveKey;
				c.MeshRef          = mc->MeshRef;       // shared GPU resource — intentional
				c.Materials        = mc->Materials;
				c.CastShadows      = mc->CastShadows;
			}
			if (const auto* cc = m_Registry.try_get<CameraComponent>(src))
			{
				auto& c    = clone->m_Registry.emplace_or_replace<CameraComponent>(dst);
				c.Camera   = cc->Camera;
				c.Primary  = cc->Primary;
			}
			if (const auto* lc = m_Registry.try_get<LightComponent>(src))
				clone->m_Registry.emplace_or_replace<LightComponent>(dst, *lc);
		}

		// Pass 3: rebuild SceneGraph hierarchy.
		for (auto [src, dst] : entityMap)
		{
			const SceneNode* node = m_SceneGraph.GetNode(static_cast<EntityID>(src));
			if (!node || node->Parent == INVALID_ENTITY)
				continue;

			auto it = entityMap.find(static_cast<entt::entity>(node->Parent));
			if (it == entityMap.end())
				continue;

			clone->m_SceneGraph.SetParent(
				static_cast<EntityID>(dst),
				static_cast<EntityID>(it->second));
		}

		return clone;
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
	
	template <>
	void Scene::OnComponentAdded<PrefabInstanceComponent>(Entity entity, PrefabInstanceComponent& component)
	{
		
	}
}
