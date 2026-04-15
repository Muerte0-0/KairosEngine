#include "kepch.h"
#include "SceneSerializer.h"

#include <yaml-cpp/yaml.h>

#include "Engine/Assets/MeshAssetManager.h"
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
	YAML::Emitter& operator<<(YAML::Emitter& emitter, const glm::vec3& v)
	{
		emitter << YAML::Flow;
		emitter << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return emitter;
	}
	
	YAML::Emitter& operator<<(YAML::Emitter& emitter, const glm::vec4& v)
	{
		emitter << YAML::Flow;
		emitter << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return emitter;
	}
	
	SceneSerializer::SceneSerializer(const Ref<Scene> scene) : m_Scene(scene) {}
	
	static void SerializeEntity(YAML::Emitter& emitter, Entity entity)
	{
		ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have an ID Component!")
		
		emitter << YAML::BeginMap; // Entity map

		// Entity ID
		emitter << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		// Tag Component
		if (entity.HasComponent<TagComponent>())
		{
			emitter << YAML::Key << "TagComponent";
			emitter << YAML::BeginMap;

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			emitter << YAML::Key << "Tag" << YAML::Value << tag;

			emitter << YAML::EndMap;
		}

		// Transform Component
		if (entity.HasComponent<TransformComponent>())
		{
			emitter << YAML::Key << "TransformComponent";
			emitter << YAML::BeginMap;

			auto& tc = entity.GetComponent<TransformComponent>();

			emitter << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			emitter << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			emitter << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			emitter << YAML::EndMap;
		}

		if (entity.HasComponent<MeshComponent>())
		{
			emitter << YAML::Key << "MeshComponent";
			emitter << YAML::BeginMap;

			auto& mc = entity.GetComponent<MeshComponent>();
			if (mc.HasMeshAsset())
				emitter << YAML::Key << "MeshAssetPath" << YAML::Value << mc.MeshAssetPath.generic_string();

			emitter << YAML::EndMap;
		}

		emitter << YAML::EndMap; // End Entity map
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter emitter;
		emitter << YAML::BeginMap; 
		emitter << YAML::Key << "Scene" << YAML::Value << "Untitled Scene"; 
		emitter << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		
		for (auto entt : std::views::reverse(m_Scene->m_Registry.view<entt::entity>()))
		{
			Entity entity { entt, m_Scene.get() };
			
			if (!entity)
				return;
			
			SerializeEntity(emitter, entity);
		}
		
		emitter << YAML::EndSeq;
		emitter << YAML::EndMap;
		
		std::ofstream fout(filepath);
		fout << emitter.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		ASSERT(false, "SceneSerializer: SerializeRuntime Not Implemented.")
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std :: string>();
		LOG(LogLevel::Trace, "Deserializing scene '{0}'", sceneName);

		if (auto entities = data["Entities"])
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();
				
				std::string name;

				if (auto tagComponent = entity["TagComponent"])
					name = tagComponent["Tag"].as<std::string>();
				
				LOG(LogLevel::Trace, "Deserializing Entity with ID: {0}, name:{1}", uuid, name);
				
				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				if (auto transformComponent = entity["TransformComponent"])
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				if (auto meshComponent = entity["MeshComponent"])
				{
					auto& mc = deserializedEntity.AddComponent<MeshComponent>();
					if (auto meshAssetPath = meshComponent["MeshAssetPath"])
					{
						std::filesystem::path assetPath = meshAssetPath.as<std::string>();
						Ref<Mesh> mesh = MeshAssetManager::GetMesh(assetPath);
						mc.SetMeshAsset(assetPath, mesh);
					}
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
