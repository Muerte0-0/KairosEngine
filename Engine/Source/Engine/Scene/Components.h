#pragma once

#include "Engine/Core/UUID.h"

#include "Engine/Renderer/Cameras/GameCamera.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <filesystem>

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
		std::filesystem::path MeshAssetPath;
		Ref<Mesh> MeshRef;

		bool SetMeshAsset(const std::filesystem::path& assetPath, const Ref<Mesh>& mesh)
		{
			if (MeshAssetPath == assetPath && MeshRef == mesh)
				return false;

			MeshAssetPath = assetPath;
			MeshRef = mesh;
			return true;
		}

		bool ClearMesh()
		{
			if (MeshAssetPath.empty() && MeshRef == nullptr)
				return false;

			MeshAssetPath.clear();
			MeshRef.reset();
			return true;
		}

		bool HasMesh() const { return MeshRef != nullptr; }
		bool HasMeshAsset() const { return !MeshAssetPath.empty(); }
	};
}
