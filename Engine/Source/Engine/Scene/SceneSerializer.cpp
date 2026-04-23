#include "kepch.h"
#include "SceneSerializer.h"

#include <yaml-cpp/yaml.h>

#include "Engine/Assets/AssetManager.h"
#include "Engine/Project/Project.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"
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
	
	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have an ID Component!")
		
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

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
			if (static_cast<uint64_t>(mc.MaterialAssetHandle) != NullAssetHandle)
				out << YAML::Key << "MaterialAssetHandle" << YAML::Value << static_cast<uint64_t>(mc.MaterialAssetHandle);
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
			SerializeEntity(out, entity);
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
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();
				std::string name;
				if (auto tc = entity["TagComponent"]) name = tc["Tag"].as<std::string>();

				LOG(LogLevel::Trace, "Deserializing Entity ID={0} name={1}", uuid, name);
				Entity deserialized = m_Scene->CreateEntityWithUUID(uuid, name);

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
					if (auto matHandleNode = meshNode["MaterialAssetHandle"])
						mc.MaterialAssetHandle = AssetHandle(matHandleNode.as<uint64_t>());
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
