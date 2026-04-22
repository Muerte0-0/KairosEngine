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

	struct MeshComponent
	{
		AssetHandle                MeshAssetHandle         = AssetHandle(NullAssetHandle);
		AssetHandle                MaterialAssetHandle     = AssetHandle(NullAssetHandle); // optional .kmat override
		Ref<Mesh>                  MeshRef;
		std::vector<Ref<Material>> Materials;   // indexed by SubMesh::MaterialIndex

		// Set mesh + materials together from an already-loaded Mesh ref.
		bool SetMeshAsset(AssetHandle handle,
		                  const Ref<Mesh>& mesh,
		                  std::vector<Ref<Material>> materials = {})
		{
			if (MeshAssetHandle == handle && MeshRef == mesh)
				return false;

			MeshAssetHandle = handle;
			MeshRef         = mesh;
			Materials       = std::move(materials);
			return true;
		}

		bool ClearMesh()
		{
			if (static_cast<uint64_t>(MeshAssetHandle) == NullAssetHandle && MeshRef == nullptr)
				return false;

			MeshAssetHandle = AssetHandle(NullAssetHandle);
			MeshRef.reset();
			Materials.clear();
			return true;
		}

		bool HasMesh()      const { return MeshRef != nullptr; }
		bool HasMeshAsset() const { return static_cast<uint64_t>(MeshAssetHandle) != NullAssetHandle; }
	};
}
