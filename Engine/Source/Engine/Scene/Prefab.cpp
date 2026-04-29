#include "kepch.h"
#include "Prefab.h"

#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "SceneGraph.h"

#include "Engine/Assets/AssetManager.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"
#include "Engine/Utils/PrimitiveMeshFactory.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <unordered_map>

// glm YAML converters (same as SceneSerializer)
namespace YAML
{
	template<> struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& v)
		{
			Node n; n.push_back(v.x); n.push_back(v.y); n.push_back(v.z); return n;
		}
		static bool decode(const Node& n, glm::vec3& v)
		{
			if (!n.IsSequence() || n.size() != 3) return false;
			v = { n[0].as<float>(), n[1].as<float>(), n[2].as<float>() };
			return true;
		}
	};
}

namespace Engine
{

static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
{
	out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

// ---------------------------------------------------------------------------
// Internal: collect entity + all descendants, depth-first
// ---------------------------------------------------------------------------
static void CollectHierarchy(Entity entity, Scene* scene, PrefabData& out)
{
	SerializedEntity se;
	se.ID  = entity.GetUUID();

	EntityID id = static_cast<entt::entity>(entity);
	uint64_t parentUUID = scene->GetParentUUID(id);
	se.ParentID = UUID(parentUUID);

	if (entity.HasComponent<TagComponent>())
		se.Tag = entity.GetComponent<TagComponent>().Tag;

	if (entity.HasComponent<TransformComponent>())
	{
		auto& tc = entity.GetComponent<TransformComponent>();
		se.Translation = tc.Translation;
		se.Rotation    = tc.Rotation;
		se.Scale       = tc.Scale;
	}

	if (entity.HasComponent<MeshComponent>())
	{
		auto& mc = entity.GetComponent<MeshComponent>();
		se.HasMesh            = true;
		se.MeshAssetHandle    = mc.MeshAssetHandle;
		se.MaterialAssetHandle= mc.MaterialAssetHandle;
		se.PrimitiveKey       = mc.PrimitiveKey;
		se.CastShadows        = mc.CastShadows;
	}

	if (entity.HasComponent<LightComponent>())
	{
		auto& lc = entity.GetComponent<LightComponent>();
		se.HasLight       = true;
		se.LightType      = static_cast<int>(lc.Type);
		se.LightColor     = lc.Color;
		se.LightIntensity = lc.Intensity;
		se.LightRange     = lc.Point.Range;
		se.SpotDirection  = lc.Spot.Direction;
		se.SpotInnerCone  = lc.Spot.InnerConeAngle;
		se.SpotOuterCone  = lc.Spot.OuterConeAngle;
	}

	if (entity.HasComponent<CameraComponent>())
	{
		auto& cc = entity.GetComponent<CameraComponent>();
		se.HasCamera     = true;
		se.CameraPrimary = cc.Primary;
		se.CameraFOV     = cc.Camera.GetFOV();
		se.CameraNear    = cc.Camera.GetNear();
		se.CameraFar     = cc.Camera.GetFar();
	}

	out.Entities.push_back(se);

	// Recurse into children
	const SceneNode* node = scene->GetSceneGraph().GetNode(id);
	if (node)
	{
		for (EntityID childID : node->Children)
		{
			Entity child{ childID, scene };
			if (child)
				CollectHierarchy(child, scene, out);
		}
	}
}

// ---------------------------------------------------------------------------
// SerializeEntityHierarchy
// ---------------------------------------------------------------------------
PrefabData SerializeEntityHierarchy(Entity root)
{
	ASSERT(root, "SerializeEntityHierarchy: invalid root entity");
	PrefabData data;
	CollectHierarchy(root, root.GetScene(), data);
	return data;
}

// ---------------------------------------------------------------------------
// SavePrefab
// ---------------------------------------------------------------------------
bool SavePrefab(const std::filesystem::path& path, const PrefabData& data)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Prefab" << YAML::Value << YAML::BeginSeq;

	for (const SerializedEntity& se : data.Entities)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity"   << YAML::Value << static_cast<uint64_t>(se.ID);
		out << YAML::Key << "ParentID" << YAML::Value << static_cast<uint64_t>(se.ParentID);

		out << YAML::Key << "TagComponent" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Tag" << YAML::Value << se.Tag;
		out << YAML::EndMap;

		out << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Translation" << YAML::Value << se.Translation;
		out << YAML::Key << "Rotation"    << YAML::Value << se.Rotation;
		out << YAML::Key << "Scale"       << YAML::Value << se.Scale;
		out << YAML::EndMap;

		if (se.HasMesh)
		{
			out << YAML::Key << "MeshComponent" << YAML::Value << YAML::BeginMap;
			if (static_cast<uint64_t>(se.MeshAssetHandle) != NullAssetHandle)
				out << YAML::Key << "MeshAssetHandle" << YAML::Value << static_cast<uint64_t>(se.MeshAssetHandle);
			if (!se.PrimitiveKey.empty())
				out << YAML::Key << "PrimitiveKey" << YAML::Value << se.PrimitiveKey;
			if (static_cast<uint64_t>(se.MaterialAssetHandle) != NullAssetHandle)
				out << YAML::Key << "MaterialAssetHandle" << YAML::Value << static_cast<uint64_t>(se.MaterialAssetHandle);
			out << YAML::Key << "CastShadows" << YAML::Value << se.CastShadows;
			out << YAML::EndMap;
		}

		if (se.HasLight)
		{
			out << YAML::Key << "LightComponent" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Type"      << YAML::Value << se.LightType;
			out << YAML::Key << "Color"     << YAML::Value << se.LightColor;
			out << YAML::Key << "Intensity" << YAML::Value << se.LightIntensity;
			out << YAML::Key << "Range"     << YAML::Value << se.LightRange;
			out << YAML::Key << "SpotDirection"  << YAML::Value << se.SpotDirection;
			out << YAML::Key << "SpotInnerCone"  << YAML::Value << se.SpotInnerCone;
			out << YAML::Key << "SpotOuterCone"  << YAML::Value << se.SpotOuterCone;
			out << YAML::EndMap;
		}

		if (se.HasCamera)
		{
			out << YAML::Key << "CameraComponent" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Primary" << YAML::Value << se.CameraPrimary;
			out << YAML::Key << "FOV"     << YAML::Value << se.CameraFOV;
			out << YAML::Key << "Near"    << YAML::Value << se.CameraNear;
			out << YAML::Key << "Far"     << YAML::Value << se.CameraFar;
			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream file(path);
	if (!file)
	{
		LOG(LogLevel::Error, "SavePrefab: failed to write '{}'.", path.string());
		return false;
	}
	file << out.c_str();
	return true;
}

// ---------------------------------------------------------------------------
// LoadPrefab
// ---------------------------------------------------------------------------
Ref<Prefab> LoadPrefab(const std::filesystem::path& path)
{
	YAML::Node root;
	try { root = YAML::LoadFile(path.string()); }
	catch (const YAML::Exception& e)
	{
		LOG(LogLevel::Error, "LoadPrefab: parse error '{}': {}", path.string(), e.what());
		return nullptr;
	}

	auto prefabNode = root["Prefab"];
	if (!prefabNode)
	{
		LOG(LogLevel::Error, "LoadPrefab: missing 'Prefab' key in '{}'.", path.string());
		return nullptr;
	}

	auto prefab = CreateRef<Prefab>();

	for (auto node : prefabNode)
	{
		SerializedEntity se;
		se.ID       = UUID(node["Entity"].as<uint64_t>(0));
		se.ParentID = UUID(node["ParentID"].as<uint64_t>(0));

		if (auto tc = node["TagComponent"])
			se.Tag = tc["Tag"].as<std::string>("");

		if (auto tr = node["TransformComponent"])
		{
			se.Translation = tr["Translation"].as<glm::vec3>(glm::vec3(0));
			se.Rotation    = tr["Rotation"].as<glm::vec3>(glm::vec3(0));
			se.Scale       = tr["Scale"].as<glm::vec3>(glm::vec3(1));
		}

		if (auto mc = node["MeshComponent"])
		{
			se.HasMesh = true;
			if (auto h = mc["MeshAssetHandle"])     se.MeshAssetHandle     = AssetHandle(h.as<uint64_t>(0));
			if (auto h = mc["MaterialAssetHandle"]) se.MaterialAssetHandle = AssetHandle(h.as<uint64_t>(0));
			if (auto h = mc["PrimitiveKey"])        se.PrimitiveKey        = h.as<std::string>("");
			se.CastShadows = mc["CastShadows"].as<bool>(true);
		}

		if (auto lc = node["LightComponent"])
		{
			se.HasLight       = true;
			se.LightType      = lc["Type"].as<int>(0);
			se.LightColor     = lc["Color"].as<glm::vec3>(glm::vec3(1));
			se.LightIntensity = lc["Intensity"].as<float>(1.0f);
			se.LightRange     = lc["Range"].as<float>(10.0f);
			if (auto d = lc["SpotDirection"]) se.SpotDirection = d.as<glm::vec3>(glm::vec3(0,-1,0));
			se.SpotInnerCone  = lc["SpotInnerCone"].as<float>(0.0f);
			se.SpotOuterCone  = lc["SpotOuterCone"].as<float>(0.0f);
		}

		if (auto cc = node["CameraComponent"])
		{
			se.HasCamera     = true;
			se.CameraPrimary = cc["Primary"].as<bool>(false);
			se.CameraFOV     = cc["FOV"].as<float>(45.0f);
			se.CameraNear    = cc["Near"].as<float>(0.1f);
			se.CameraFar     = cc["Far"].as<float>(1000.0f);
		}

		prefab->Data.Entities.push_back(se);
	}

	return prefab;
}


// ---------------------------------------------------------------------------
// Prefab::Instantiate
// ---------------------------------------------------------------------------
Entity Prefab::Instantiate(Scene* scene) const
{
	ASSERT(scene, "Prefab::Instantiate: scene is null");

	if (Data.Entities.empty())
	{
		LOG(LogLevel::Warning, "Prefab::Instantiate: prefab has no entities.");
		return {};
	}

	// Pass 1 — generate new UUIDs, create entities
	// idMap: prefab UUID → new UUID
	std::unordered_map<uint64_t, UUID>     idMap;
	std::unordered_map<uint64_t, EntityID> uuidToEntityID;

	for (const SerializedEntity& se : Data.Entities)
	{
		UUID newID;
		idMap[static_cast<uint64_t>(se.ID)] = newID;

		Entity e = scene->CreateEntityWithUUID(newID, se.Tag);
		uuidToEntityID[static_cast<uint64_t>(se.ID)] = static_cast<EntityID>((entt::entity)e);
	}

	// Pass 2 — copy components
	for (const SerializedEntity& se : Data.Entities)
	{
		auto it = uuidToEntityID.find(static_cast<uint64_t>(se.ID));
		if (it == uuidToEntityID.end()) continue;

		Entity e{ it->second, scene };

		// Transform (always present via CreateEntityWithUUID)
		auto& tc = e.GetComponent<TransformComponent>();
		tc.Translation = se.Translation;
		tc.Rotation    = se.Rotation;
		tc.Scale       = se.Scale;

		if (se.HasMesh)
		{
			auto& mc = e.AddComponent<MeshComponent>();
			mc.MeshAssetHandle     = se.MeshAssetHandle;
			mc.MaterialAssetHandle = se.MaterialAssetHandle;
			mc.PrimitiveKey        = se.PrimitiveKey;
			mc.CastShadows         = se.CastShadows;

			// Resolve MeshRef immediately — instantiate runs on main thread mid-session,
			// not during startup, so LoadingSystem pump is not available.
			if (static_cast<uint64_t>(se.MeshAssetHandle) != NullAssetHandle)
			{
				Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(se.MeshAssetHandle);
				if (mesh)
					mc.SetMeshAsset(se.MeshAssetHandle, mesh, mesh->GetMaterials());
				else
					LOG(LogLevel::Warning, "Prefab::Instantiate: could not resolve Mesh asset {}.", static_cast<uint64_t>(se.MeshAssetHandle));
			}
			else if (!se.PrimitiveKey.empty())
			{
				Ref<Mesh> mesh = PrimitiveMeshFactory::GetOrCreate(se.PrimitiveKey);
				if (mesh)
					mc.SetPrimitiveMesh(se.PrimitiveKey, mesh);
				else
					LOG(LogLevel::Warning, "Prefab::Instantiate: could not resolve primitive '{}'.", se.PrimitiveKey);
			}
		}

		if (se.HasLight)
		{
			auto& lc   = e.AddComponent<LightComponent>();
			lc.Type      = static_cast<LightType>(se.LightType);
			lc.Color     = se.LightColor;
			lc.Intensity = se.LightIntensity;
			lc.Point.Range        = se.LightRange;
			lc.Spot.Direction     = se.SpotDirection;
			lc.Spot.Range         = se.LightRange;
			lc.Spot.InnerConeAngle= se.SpotInnerCone;
			lc.Spot.OuterConeAngle= se.SpotOuterCone;
		}

		if (se.HasCamera)
		{
			auto& cc = e.AddComponent<CameraComponent>();
			cc.Primary = se.CameraPrimary;
			cc.Camera.SetFOV(se.CameraFOV);
			cc.Camera.SetNearFar(se.CameraNear, se.CameraFar);
		}
	}

	// Pass 3 — rebuild hierarchy using idMap
	for (const SerializedEntity& se : Data.Entities)
	{
		uint64_t oldParentUUID = static_cast<uint64_t>(se.ParentID);
		if (oldParentUUID == 0) continue;

		auto childIt  = uuidToEntityID.find(static_cast<uint64_t>(se.ID));
		auto parentIt = uuidToEntityID.find(oldParentUUID);

		if (childIt != uuidToEntityID.end() && parentIt != uuidToEntityID.end())
			scene->GetSceneGraph().SetParent(childIt->second, parentIt->second);
	}

	// Find root (ParentID == 0) and attach PrefabInstanceComponent
	Entity rootEntity;
	for (const SerializedEntity& se : Data.Entities)
	{
		if (static_cast<uint64_t>(se.ParentID) == 0)
		{
			auto it = uuidToEntityID.find(static_cast<uint64_t>(se.ID));
			if (it != uuidToEntityID.end())
			{
				rootEntity = Entity{ it->second, scene };
				break;
			}
		}
	}

	if (rootEntity)
		rootEntity.AddComponent<PrefabInstanceComponent>(Handle);

	return rootEntity;
}

} // namespace Engine
