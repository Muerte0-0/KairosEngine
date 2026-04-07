#pragma once

#include <glm/glm.hpp>

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
		glm::mat4 Transform { 1.0f };
		
		TransformComponent() = default;
		TransformComponent(const glm::mat4& transform) : Transform(transform) {}
		
		operator glm::mat4&() { return Transform; }
		operator const glm::mat4&() const { return Transform; }
	};

	struct TickableComponent
	{
		float DeltaTime = 0.0f;
		
		void OnUpdate(float deltaTime)
		{
			DeltaTime = deltaTime;
		}
	};

	struct CameraComponent
	{
		Engine::GameCamera Camera;
		bool Primary = false;
		
		CameraComponent() = default;
	};

	struct MeshComponent
	{
		Ref<Mesh> Mesh;
	};
}
