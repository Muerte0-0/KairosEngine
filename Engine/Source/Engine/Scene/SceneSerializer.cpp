#include "kepch.h"
#include "SceneSerializer.h"

#include <yaml-cpp/yaml.h>

#include "Engine/Assets/AssetManager.h"
#include "Engine/Project/Project.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"
#include "Engine/Utils/PrimitiveMeshFactory.h"
#include "Engine/Core/LoadingSystem.h"
#include "Components.h"
#include "Entity.h"
#include "Prefab.h"

namespace YAML
{
	template <>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}
		
		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() && node.size() != 3)
				return false;
			
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};
	
	template <>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}
		
		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() && node.size() != 4)
				return false;
			
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

namespace Engine
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	static bool HasPrefabInstanceAncestor(Entity entity, Scene* scene)
	{
		EntityID id = entity;
		const SceneNode* node = scene->GetSceneGraph().GetNode(id);
		if (!node)
			return false;

		id = node->Parent;
		while (id != INVALID_ENTITY)
		{
			Entity ancestor{ static_cast<entt::entity>(id), scene };
			if (ancestor && ancestor.HasComponent<PrefabInstanceComponent>())
				return true;

			node = scene->GetSceneGraph().GetNode(id);
			if (!node)
				break;
			id = node->Parent;
		}

		return false;
	}

	static bool IsPrefabOverride(Entity entity, ComponentType type)
	{
		if (!entity.HasComponent<PrefabOverrideComponent>())
			return false;
		return entity.GetComponent<PrefabOverrideComponent>().OverriddenComponents.contains(type);
	}

	static void SerializeTagComponent(YAML::Emitter& out, Entity entity)
	{
		out << YAML::Key << "TagComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Tag" << YAML::Value << entity.GetComponent<TagComponent>().Tag;
		out << YAML::EndMap;
	}

	static void SerializeTransformComponent(YAML::Emitter& out, Entity entity)
	{
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap;
		auto& tc = entity.GetComponent<TransformComponent>();
		out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
		out << YAML::Key << "Rotation"    << YAML::Value << tc.Rotation;
		out << YAML::Key << "Scale"       << YAML::Value << tc.Scale;
		out << YAML::EndMap;
	}

	static void SerializeMeshComponent(YAML::Emitter& out, Entity entity)
	{
		out << YAML::Key << "MeshComponent";
		out << YAML::BeginMap;
		auto& mc = entity.GetComponent<MeshComponent>();
		if (mc.HasMeshAsset())
			out << YAML::Key << "MeshAssetHandle" << YAML::Value << static_cast<uint64_t>(mc.MeshAssetHandle);
		if (mc.IsPrimitive())
			out << YAML::Key << "PrimitiveKey" << YAML::Value << mc.PrimitiveKey;
		if (static_cast<uint64_t>(mc.MaterialAssetHandle) != NullAssetHandle)
			out << YAML::Key << "MaterialAssetHandle" << YAML::Value << static_cast<uint64_t>(mc.MaterialAssetHandle);
		out << YAML::Key << "CastShadows" << YAML::Value << mc.CastShadows;
		out << YAML::EndMap;
	}

	static void SerializeLightComponent(YAML::Emitter& out, Entity entity)
	{
		out << YAML::Key << "LightComponent";
		out << YAML::BeginMap;
		auto& lc = entity.GetComponent<LightComponent>();
		out << YAML::Key << "Type"      << YAML::Value << static_cast<int>(lc.Type);
		out << YAML::Key << "Color"     << YAML::Value << lc.Color;
		out << YAML::Key << "Intensity" << YAML::Value << lc.Intensity;

		if (lc.Type == LightType::Point)
			out << YAML::Key << "Range" << YAML::Value << lc.Point.Range;

		if (lc.Type == LightType::Spot)
		{
			out << YAML::Key << "Direction"      << YAML::Value << lc.Spot.Direction;
			out << YAML::Key << "Range"          << YAML::Value << lc.Spot.Range;
			out << YAML::Key << "InnerConeAngle" << YAML::Value << lc.Spot.InnerConeAngle;
			out << YAML::Key << "OuterConeAngle" << YAML::Value << lc.Spot.OuterConeAngle;
		}

		out << YAML::EndMap;
	}

	static void SerializeCameraComponent(YAML::Emitter& out, Entity entity)
	{
		out << YAML::Key << "CameraComponent";
		out << YAML::BeginMap;
		auto& cc = entity.GetComponent<CameraComponent>();
		out << YAML::Key << "Primary"  << YAML::Value << cc.Primary;
		out << YAML::Key << "FOV"      << YAML::Value << cc.Camera.GetFOV();
		out << YAML::Key << "Near"     << YAML::Value << cc.Camera.GetNear();
		out << YAML::Key << "Far"      << YAML::Value << cc.Camera.GetFar();
		out << YAML::EndMap;
	}
	
	SceneSerializer::SceneSerializer(const Ref<Scene> scene) : m_Scene(scene) {}
	
	static void SerializeEntity(YAML::Emitter& out, Entity entity, Scene* scene)
	{
		ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have an ID Component!")
		
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		// --- Hierarchy ---
		{
			EntityID id = entity;
			uint64_t parentUUID = scene->GetParentUUID(id);
			out << YAML::Key << "ParentUUID" << YAML::Value << parentUUID;
		}

		const bool isPrefabInstance = entity.HasComponent<PrefabInstanceComponent>();
		if (isPrefabInstance)
		{
			out << YAML::Key << "PrefabInstanceComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "PrefabHandle" << YAML::Value << static_cast<uint64_t>(entity.GetComponent<PrefabInstanceComponent>().PrefabHandle);
			out << YAML::EndMap;

			if (entity.HasComponent<PrefabOverrideComponent>())
			{
				out << YAML::Key << "PrefabOverrideComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "OverriddenComponents" << YAML::Value << YAML::BeginSeq;
				for (ComponentType type : entity.GetComponent<PrefabOverrideComponent>().OverriddenComponents)
					out << ComponentTypeToString(type);
				out << YAML::EndSeq;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<TagComponent>()       && IsPrefabOverride(entity, ComponentType::Tag))       SerializeTagComponent(out, entity);
			if (entity.HasComponent<TransformComponent>() && IsPrefabOverride(entity, ComponentType::Transform)) SerializeTransformComponent(out, entity);
			if (entity.HasComponent<MeshComponent>()      && IsPrefabOverride(entity, ComponentType::Mesh))      SerializeMeshComponent(out, entity);
			if (entity.HasComponent<LightComponent>()     && IsPrefabOverride(entity, ComponentType::Light))     SerializeLightComponent(out, entity);
			if (entity.HasComponent<CameraComponent>()    && IsPrefabOverride(entity, ComponentType::Camera))    SerializeCameraComponent(out, entity);
			out << YAML::EndMap;
			return;
		}

		if (entity.HasComponent<TagComponent>())       SerializeTagComponent(out, entity);
		if (entity.HasComponent<TransformComponent>()) SerializeTransformComponent(out, entity);
		if (entity.HasComponent<MeshComponent>())      SerializeMeshComponent(out, entity);
		if (entity.HasComponent<LightComponent>())     SerializeLightComponent(out, entity);
		if (entity.HasComponent<CameraComponent>())    SerializeCameraComponent(out, entity);

		out << YAML::EndMap;
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; 
		out << YAML::Key << "Scene"    << YAML::Value << m_Scene->GetName(); 
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		
		for (auto entt : std::views::reverse(m_Scene->m_Registry.view<entt::entity>()))
		{
			Entity entity { entt, m_Scene.get() };
			if (!entity) return;
			if (HasPrefabInstanceAncestor(entity, m_Scene.get()))
				continue;
			SerializeEntity(out, entity, m_Scene.get());
		}
		
		out << YAML::EndSeq;
		out << YAML::EndMap;
		
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		ASSERT(false, "SceneSerializer: SerializeRuntime Not Implemented.")
	}

	static std::unordered_set<ComponentType> ReadPrefabOverrideTypes(const YAML::Node& entityNode)
	{
		std::unordered_set<ComponentType> types;
		if (auto overrideNode = entityNode["PrefabOverrideComponent"])
		{
			if (auto list = overrideNode["OverriddenComponents"])
			{
				for (auto typeNode : list)
				{
					ComponentType type;
					if (ComponentTypeFromString(typeNode.as<std::string>(""), type))
						types.insert(type);
				}
			}
		}
		return types;
	}

	static void ApplySerializedComponentOverrides(Entity entity, const YAML::Node& entityNode, const std::unordered_set<ComponentType>& overrides)
	{
		auto& registry = entity.GetScene()->GetRegistry();
		auto enttID = static_cast<entt::entity>(entity);

		if (overrides.contains(ComponentType::Tag))
		{
			if (auto tagNode = entityNode["TagComponent"])
				entity.GetComponent<TagComponent>().Tag = tagNode["Tag"].as<std::string>("");
		}

		if (overrides.contains(ComponentType::Transform))
		{
			if (auto transformNode = entityNode["TransformComponent"])
			{
				auto& tc = entity.GetComponent<TransformComponent>();
				tc.Translation = transformNode["Translation"].as<glm::vec3>();
				tc.Rotation    = transformNode["Rotation"].as<glm::vec3>();
				tc.Scale       = transformNode["Scale"].as<glm::vec3>();
			}
		}

		if (overrides.contains(ComponentType::Mesh))
		{
			if (auto meshNode = entityNode["MeshComponent"])
			{
				auto& mc = registry.emplace_or_replace<MeshComponent>(enttID);
				if (auto handleNode = meshNode["MeshAssetHandle"])
				{
					AssetHandle handle(handleNode.as<uint64_t>());
					mc.MeshAssetHandle = handle;
					Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(handle);
					if (mesh)
						mc.SetMeshAsset(handle, mesh, mesh->GetMaterials());
				}
				else if (auto primNode = meshNode["PrimitiveKey"])
				{
					std::string key = primNode.as<std::string>();
					mc.SetPrimitiveMesh(key, PrimitiveMeshFactory::GetOrCreate(key));
				}
				if (auto matHandleNode = meshNode["MaterialAssetHandle"])
					mc.MaterialAssetHandle = AssetHandle(matHandleNode.as<uint64_t>());
				if (auto castShadowsNode = meshNode["CastShadows"])
					mc.CastShadows = castShadowsNode.as<bool>();
			}
			else if (entity.HasComponent<MeshComponent>())
			{
				entity.RemoveComponent<MeshComponent>();
			}
		}

		if (overrides.contains(ComponentType::Light))
		{
			if (auto lightNode = entityNode["LightComponent"])
			{
				auto& lc = registry.emplace_or_replace<LightComponent>(enttID);
				lc.Type      = static_cast<LightType>(lightNode["Type"].as<int>());
				lc.Color     = lightNode["Color"].as<glm::vec3>();
				lc.Intensity = lightNode["Intensity"].as<float>();
				if (lc.Type == LightType::Point)
				{
					if (auto n = lightNode["Range"]) lc.Point.Range = n.as<float>();
				}
				else if (lc.Type == LightType::Spot)
				{
					if (auto n = lightNode["Direction"])      lc.Spot.Direction      = n.as<glm::vec3>();
					if (auto n = lightNode["Range"])          lc.Spot.Range          = n.as<float>();
					if (auto n = lightNode["InnerConeAngle"]) lc.Spot.InnerConeAngle = n.as<float>();
					if (auto n = lightNode["OuterConeAngle"]) lc.Spot.OuterConeAngle = n.as<float>();
				}
			}
			else if (entity.HasComponent<LightComponent>())
			{
				entity.RemoveComponent<LightComponent>();
			}
		}

		if (overrides.contains(ComponentType::Camera))
		{
			if (auto camNode = entityNode["CameraComponent"])
			{
				auto& cc = registry.emplace_or_replace<CameraComponent>(enttID);
				cc.Primary = camNode["Primary"] ? camNode["Primary"].as<bool>() : false;
				float fov  = camNode["FOV"]  ? camNode["FOV"].as<float>()  : 45.0f;
				float nearPlane = camNode["Near"] ? camNode["Near"].as<float>() : 0.1f;
				float farPlane  = camNode["Far"]  ? camNode["Far"].as<float>()  : 1000.0f;
				cc.Camera.SetFOV(fov);
				cc.Camera.SetNearFar(nearPlane, farPlane);
			}
			else if (entity.HasComponent<CameraComponent>())
			{
				entity.RemoveComponent<CameraComponent>();
			}
		}
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try   { data = YAML::LoadFile(filepath); }
		catch (YAML::ParserException e)
		{
			LOG(LogLevel::Error, "Failed to Load Scene: '{0}'\n {1}", filepath, e.what());
			return false;
		}
		
		if (!data["Scene"]) return false;

		m_Scene->SetName(data["Scene"].as<std::string>());
		
		LOG(LogLevel::Trace, "Deserializing scene '{0}'", m_Scene->GetName());

		if (auto entities = data["Entities"])
		{
		// --- Pass 1: create all entities ---
		// Map UUID → entt handle for parent wiring
		std::unordered_map<uint64_t, EntityID> uuidToEntityID;

		for (auto entity : entities)
		{
			uint64_t uuid = entity["Entity"].as<uint64_t>();
			std::string name;
			if (auto tc = entity["TagComponent"]) name = tc["Tag"].as<std::string>();

			LOG(LogLevel::Trace, "Deserializing Entity ID={0} name={1}", uuid, name);

			if (auto prefabNode = entity["PrefabInstanceComponent"])
			{
				AssetHandle prefabHandle(prefabNode["PrefabHandle"].as<uint64_t>(NullAssetHandle));
				Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabHandle);
				if (!prefab)
				{
					LOG(LogLevel::Warning, "SceneSerializer: could not resolve prefab {} for instance {}", static_cast<uint64_t>(prefabHandle), uuid);
					continue;
				}

				Entity deserialized = prefab->Instantiate(m_Scene.get());
				if (!deserialized)
					continue;

				deserialized.GetComponent<IDComponent>().ID = UUID(uuid);
				if (auto tagNode = entity["TagComponent"])
					deserialized.GetComponent<TagComponent>().Tag = tagNode["Tag"].as<std::string>(name);

				auto overrides = ReadPrefabOverrideTypes(entity);
				ApplySerializedComponentOverrides(deserialized, entity, overrides);
				if (!overrides.empty())
				{
					if (!deserialized.HasComponent<PrefabOverrideComponent>())
						deserialized.AddComponent<PrefabOverrideComponent>();
					deserialized.GetComponent<PrefabOverrideComponent>().OverriddenComponents = overrides;
				}

				uuidToEntityID[uuid] = static_cast<EntityID>((entt::entity)deserialized);
				continue;
			}

			Entity deserialized = m_Scene->CreateEntityWithUUID(uuid, name);
			uuidToEntityID[uuid] = static_cast<EntityID>((entt::entity)deserialized);

			if (auto transformNode = entity["TransformComponent"])
			{
				auto& tc = deserialized.GetComponent<TransformComponent>();
				tc.Translation = transformNode["Translation"].as<glm::vec3>();
				tc.Rotation    = transformNode["Rotation"].as<glm::vec3>();
				tc.Scale       = transformNode["Scale"].as<glm::vec3>();
			}

			if (auto meshNode = entity["MeshComponent"])
			{
				auto& mc = deserialized.AddComponent<MeshComponent>();
				if (auto handleNode = meshNode["MeshAssetHandle"])
				{
					AssetHandle handle(handleNode.as<uint64_t>());
					// Defer asset import + GPU upload to main thread incremental startup pump
					// so loading screen can keep updating.
					mc.MeshAssetHandle = handle;
					LoadingSystem::EnqueueStartupMeshAsset(handle);
				}
				else if (auto primNode = meshNode["PrimitiveKey"])
				{
					std::string key = primNode.as<std::string>();
					// Primitive meshes also upload on creation; defer to startup pump.
					mc.PrimitiveKey = key;
					LoadingSystem::EnqueueStartupPrimitive(key);
				}
				if (auto matHandleNode = meshNode["MaterialAssetHandle"])
					mc.MaterialAssetHandle = AssetHandle(matHandleNode.as<uint64_t>());
				if (auto castShadowsNode = meshNode["CastShadows"])
					mc.CastShadows = castShadowsNode.as<bool>();
			}

			if (auto lightNode = entity["LightComponent"])
			{
				auto& lc    = deserialized.AddComponent<LightComponent>();
				lc.Type      = static_cast<LightType>(lightNode["Type"].as<int>());
				lc.Color     = lightNode["Color"].as<glm::vec3>();
				lc.Intensity = lightNode["Intensity"].as<float>();

				if (lc.Type == LightType::Directional)
				{
					// Direction driven by entity rotation — no fields to read.
				}
				else if (lc.Type == LightType::Point)
				{
					if (auto n = lightNode["Range"]) lc.Point.Range = n.as<float>();
				}
				else if (lc.Type == LightType::Spot)
				{
					if (auto n = lightNode["Direction"])      lc.Spot.Direction      = n.as<glm::vec3>();
					if (auto n = lightNode["Range"])          lc.Spot.Range          = n.as<float>();
					if (auto n = lightNode["InnerConeAngle"]) lc.Spot.InnerConeAngle = n.as<float>();
					if (auto n = lightNode["OuterConeAngle"]) lc.Spot.OuterConeAngle = n.as<float>();
				}
			}

			if (auto camNode = entity["CameraComponent"])
			{
				auto& cc = deserialized.AddComponent<CameraComponent>();
				cc.Primary = camNode["Primary"] ? camNode["Primary"].as<bool>() : false;
				float fov  = camNode["FOV"]  ? camNode["FOV"].as<float>()  : 45.0f;
				float nearPlane = camNode["Near"] ? camNode["Near"].as<float>() : 0.1f;
				float farPlane  = camNode["Far"]  ? camNode["Far"].as<float>()  : 1000.0f;
				cc.Camera.SetFOV(fov);
				cc.Camera.SetNearFar(nearPlane, farPlane);
			}
		}

		// --- Pass 2: wire parent relationships ---
		for (auto entity : entities)
		{
			uint64_t uuid       = entity["Entity"].as<uint64_t>();
			uint64_t parentUUID = 0;
			if (auto p = entity["ParentUUID"]) parentUUID = p.as<uint64_t>();

			if (parentUUID != 0)
			{
				auto childIt  = uuidToEntityID.find(uuid);
				auto parentIt = uuidToEntityID.find(parentUUID);
				if (childIt != uuidToEntityID.end() && parentIt != uuidToEntityID.end())
					m_Scene->GetSceneGraph().SetParent(childIt->second, parentIt->second);
				else
					LOG(LogLevel::Warning, "SceneSerializer: could not resolve parent {} for entity {}", parentUUID, uuid);
			}
		}
		}
		
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		ASSERT(false, "SceneSerializer: DeserializeRuntime Not Implemented.")
		return false;
	}
}
