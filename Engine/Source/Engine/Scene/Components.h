#pragma once

#include "Engine/Core/UUID.h"
#include "Engine/Assets/Asset.h"

#include "Engine/Renderer/Cameras/GameCamera.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"
#include "Engine/Renderer/RHI/Resources/Material.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Engine
{
	struct IDComponent
	{
		UUID ID;
		
		IDComponent() = default;
		IDComponent(const UUID& id) : ID(id) {}
	};
	struct TagComponent
	{
		std::string Tag;
		
		TagComponent() = default;
		TagComponent(const std::string& tag) : Tag(tag) {}
	};
	
	struct TransformComponent
	{
		glm::vec3 Translation	= { 0.0f, 0.0f, 0.0f};
		glm::vec3 Rotation		= { 0.0f, 0.0f, 0.0f};
		glm::vec3 Scale			= { 1.0f, 1.0f, 1.0f};

		// Computed by SceneGraph transform propagation — do not set manually.
		glm::mat4 WorldTransform = glm::mat4(1.0f);

		TransformComponent() = default;
		TransformComponent(const glm::vec3& translation) : Translation(translation) {}
		
		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			
			return glm::translate(glm::mat4(1.0f), Translation) 
				* rotation 
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct TickableComponent
	{
		virtual ~TickableComponent() = default;
		float DeltaTime = 0.0f;
		
		virtual void OnUpdate(float deltaTime)
		{
			DeltaTime = deltaTime;
		}
	};

	struct CameraComponent : public TickableComponent
	{
		Engine::GameCamera Camera;
		bool Primary = false;
		
		CameraComponent() = default;
		
		//void OnUpdate(float deltaTime) override;
	};

	// -------------------------------------------------------------------------
	// Light Component
	// -------------------------------------------------------------------------
	enum class LightType : uint8_t
	{
		Directional = 0,
		Point,
		Spot
	};

	struct DirectionalLightData
	{
	};

	struct PointLightData
	{
		float Range = 10.0f;
	};

	struct SpotLightData
	{
		glm::vec3	Direction		= { 0.0f, -1.0f, 0.0f }; // normalized, world-space
		float		Range			= 10.0f;
		float		InnerConeAngle	= glm::radians(15.0f);    // radians
		float		OuterConeAngle	= glm::radians(30.0f);    // radians
	};

	struct LightComponent
	{
		LightType	Type		= LightType::Directional;
		glm::vec3	Color		= { 1.0f, 1.0f, 1.0f };
		float		Intensity	= 1.0f;

		DirectionalLightData Directional;
		PointLightData       Point;
		SpotLightData        Spot;

		LightComponent() = default;
	};

	struct MeshComponent
	{
		AssetHandle                MeshAssetHandle     = AssetHandle(NullAssetHandle);
		AssetHandle                MaterialAssetHandle = AssetHandle(NullAssetHandle); // optional .kmat override
		std::string                PrimitiveKey;  // non-empty when mesh is a built-in primitive
		Ref<Mesh>                  MeshRef;
		std::vector<Ref<Material>> Materials;     // indexed by SubMesh::MaterialIndex
		bool                       CastShadows   = true;

		// Set mesh + materials together from an already-loaded Mesh ref.
		bool SetMeshAsset(AssetHandle handle, const Ref<Mesh>& mesh, std::vector<Ref<Material>> materials = {})
		{
			if (MeshAssetHandle == handle && MeshRef == mesh)
				return false;

			MeshAssetHandle = handle;
			PrimitiveKey.clear();
			MeshRef         = mesh;
			Materials       = std::move(materials);
			return true;
		}

		// Set a built-in primitive mesh by key (see PrimitiveKey::*).
		bool SetPrimitiveMesh(const std::string& key, const Ref<Mesh>& mesh)
		{
			if (PrimitiveKey == key && MeshRef == mesh)
				return false;

			PrimitiveKey    = key;
			MeshAssetHandle = AssetHandle(NullAssetHandle);
			MeshRef         = mesh;
			Materials.clear();
			return true;
		}

		bool ClearMesh()
		{
			if (static_cast<uint64_t>(MeshAssetHandle) == NullAssetHandle && PrimitiveKey.empty() && MeshRef == nullptr)
				return false;

			MeshAssetHandle = AssetHandle(NullAssetHandle);
			PrimitiveKey.clear();
			MeshRef.reset();
			Materials.clear();
			return true;
		}

		bool HasMesh()        const { return MeshRef != nullptr; }
		bool HasMeshAsset()   const { return static_cast<uint64_t>(MeshAssetHandle) != NullAssetHandle; }
		bool IsPrimitive()    const { return !PrimitiveKey.empty(); }
	};
}
