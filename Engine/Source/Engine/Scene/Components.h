#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Engine/Renderer/Cameras/GameCamera.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"

namespace Engine
{
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
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1.0f, 0.0f, 0.0f })
							* glm::rotate(glm::mat4(1.0f), Rotation.y, { 0.0f, 1.0f, 0.0f })
							* glm::rotate(glm::mat4(1.0f), Rotation.z, { 0.0f, 0.0f, 1.0f });
			
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
		Ref<Mesh> Mesh;
		
		bool HasMesh() const { return Mesh != nullptr; }
	};
}
