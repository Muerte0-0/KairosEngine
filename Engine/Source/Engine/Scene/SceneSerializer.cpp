#include "kepch.h"
#include "SceneSerializer.h"

#include <yaml-cpp/yaml.h>

#include "Engine/Assets/AssetManager.h"
#include "Engine/Project/Project.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"
#include "Engine/Utils/PrimitiveMeshFactory.h"
#include "Components.h"
#include "Entity.h"

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

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Tag" << YAML::Value << entity.GetComponent<TagComponent>().Tag;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;
			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation"    << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale"       << YAML::Value << tc.Scale;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<MeshComponent>())
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

		if (entity.HasComponent<LightComponent>())
		{
			out << YAML::Key << "LightComponent";
			out << YAML::BeginMap;
			auto& lc = entity.GetComponent<LightComponent>();
			out << YAML::Key << "Type"      << YAML::Value << static_cast<int>(lc.Type);
			out << YAML::Key << "Color"     << YAML::Value << lc.Color;
			out << YAML::Key << "Intensity" << YAML::Value << lc.Intensity;

			if (lc.Type == LightType::Directional)
			{
				// Direction is derived from entity rotation at runtime — nothing to serialize.
			}

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

		out << YAML::EndMap;
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; 
		out << YAML::Key << "Scene"    << YAML::Value << "Untitled Scene"; 
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		
		for (auto entt : std::views::reverse(m_Scene->m_Registry.view<entt::entity>()))
		{
			Entity entity { entt, m_Scene.get() };
			if (!entity) return;
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

		LOG(LogLevel::Trace, "Deserializing scene '{0}'", data["Scene"].as<std::string>());

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
					Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(handle);
					if (mesh)
						mc.SetMeshAsset(handle, mesh, mesh->GetMaterials());
					else
						LOG(LogLevel::Warning, "SceneSerializer: handle {} not resolvable — asset missing?", handleNode.as<uint64_t>());
				}
				else if (auto primNode = meshNode["PrimitiveKey"])
				{
					std::string key = primNode.as<std::string>();
					Ref<Mesh> mesh = PrimitiveMeshFactory::GetOrCreate(key);
					if (mesh)
						mc.SetPrimitiveMesh(key, mesh);
					else
						LOG(LogLevel::Warning, "SceneSerializer: unknown primitive key '{}'", key);
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
