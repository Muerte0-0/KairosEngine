#pragma once

#include "Scene.h"

#include "entt.hpp"

namespace Engine
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entity, Scene* scene);
		
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			ASSERT(!HasComponent<T>(), "Entity already Has Component");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...); 
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}
		
		template<typename T>
		T& GetComponent()
		{
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}
		
		template<typename T>
		bool HasComponent() const
		{
			return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
		}
		
		template<typename T>
		void RemoveComponent() const
		{
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}
		
		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return static_cast<uint32_t>(m_EntityHandle); }
		
		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return *this == other; }
	private:
		entt::entity m_EntityHandle{ entt::null };
		
		Scene* m_Scene = nullptr;
	};
}
