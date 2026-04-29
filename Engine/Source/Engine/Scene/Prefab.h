#pragma once
#include "Engine/Assets/Asset.h"
#include "Engine/Core/UUID.h"
#include <filesystem>
#include <vector>
#include <glm/glm.hpp>

namespace Engine
{
	class Scene;
	class Entity;

	// Serialized representation of a single entity in a prefab hierarchy.
	// Mirrors the fields SceneSerializer writes per entity.
	struct SerializedEntity
	{
		UUID   ID       = UUID(0);
		UUID   ParentID = UUID(0); // 0 = root
		std::string Tag;

		// TransformComponent
		glm::vec3 Translation = { 0, 0, 0 };
		glm::vec3 Rotation    = { 0, 0, 0 };
		glm::vec3 Scale       = { 1, 1, 1 };

		// Optional components — null handle = not present
		bool        HasMesh             = false;
		AssetHandle MeshAssetHandle     = AssetHandle(NullAssetHandle);
		AssetHandle MaterialAssetHandle = AssetHandle(NullAssetHandle);
		std::string PrimitiveKey;
		bool        CastShadows         = true;

		bool      HasLight    = false;
		int       LightType   = 0; // LightType enum value
		glm::vec3 LightColor  = { 1, 1, 1 };
		float     LightIntensity = 1.0f;
		float     LightRange  = 10.0f; // Point / Spot
		glm::vec3 SpotDirection     = { 0, -1, 0 };
		float     SpotInnerCone     = 0.0f;
		float     SpotOuterCone     = 0.0f;

		bool  HasCamera   = false;
		bool  CameraPrimary = false;
		float CameraFOV   = 45.0f;
		float CameraNear  = 0.1f;
		float CameraFar   = 1000.0f;
	};

	struct PrefabData
	{
		// Flat list — order determines pass-1 creation order.
		// Entity with ParentID == 0 is the root.
		std::vector<SerializedEntity> Entities;
	};

	// Serialize a live entity + its full descendant hierarchy into PrefabData.
	// Does NOT modify the scene. Does NOT require saving.
	PrefabData SerializeEntityHierarchy(Entity root);

	class Prefab : public Asset
	{
	public:
		PrefabData Data;

		AssetType GetType() const override { return AssetType::Prefab; }

		// Instantiate the prefab hierarchy into scene.
		// Returns the root entity. Generates new UUIDs for all entities.
		// Attaches PrefabInstanceComponent to the root.
		Entity Instantiate(Scene* scene) const;
	};

	// Save PrefabData to a .prefab YAML file.
	// Returns false on IO failure.
	bool SavePrefab(const std::filesystem::path& path, const PrefabData& data);

	// Load a .prefab file into a Prefab asset.
	// Returns nullptr on parse/IO failure.
	Ref<Prefab> LoadPrefab(const std::filesystem::path& path);

} // namespace Engine
