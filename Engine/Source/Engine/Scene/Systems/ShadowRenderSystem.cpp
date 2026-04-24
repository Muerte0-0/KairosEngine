#include "kepch.h"
#include "ShadowRenderSystem.h"

#include "Engine/Renderer/ShadowPass.h"
#include "Engine/Scene/Components.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Engine::ShadowRenderSystem
{
	namespace
	{
		struct Bounds
		{
			glm::vec3 Min{ FLT_MAX, FLT_MAX, FLT_MAX };
			glm::vec3 Max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

			bool IsValid() const { return Min.x <= Max.x; }

			void Expand(const glm::vec3& point)
			{
				Min = glm::min(Min, point);
				Max = glm::max(Max, point);
			}
		};

		glm::vec3 ComputeDown(const TransformComponent& transform)
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(transform.Rotation));
			return glm::normalize(glm::vec3(rotation * glm::vec4(0.f, -1.f, 0.f, 0.f)));
		}

		bool ShouldCastShadow(entt::registry& registry, entt::entity entity)
		{
			auto taggedCasters = registry.view<ShadowCasterComponent>();
			return taggedCasters.empty() || registry.all_of<ShadowCasterComponent>(entity);
		}

		Bounds ComputeCasterBounds(entt::registry& registry)
		{
			Bounds bounds;
			auto view = registry.view<MeshComponent, TransformComponent>();
			for (auto entity : view)
			{
				if (!ShouldCastShadow(registry, entity))
					continue;

				const auto& [meshComponent, transformComponent] = view.get<MeshComponent, TransformComponent>(entity);
				if (!meshComponent.HasMesh())
					continue;

				const Mesh::AABB localBounds = meshComponent.MeshRef->ComputeAABB();
				if (!localBounds.IsValid())
					continue;

				const glm::mat4 model = transformComponent.GetTransform();
				const glm::vec3 corners[8] = {
					{ localBounds.Min.x, localBounds.Min.y, localBounds.Min.z },
					{ localBounds.Max.x, localBounds.Min.y, localBounds.Min.z },
					{ localBounds.Min.x, localBounds.Max.y, localBounds.Min.z },
					{ localBounds.Max.x, localBounds.Max.y, localBounds.Min.z },
					{ localBounds.Min.x, localBounds.Min.y, localBounds.Max.z },
					{ localBounds.Max.x, localBounds.Min.y, localBounds.Max.z },
					{ localBounds.Min.x, localBounds.Max.y, localBounds.Max.z },
					{ localBounds.Max.x, localBounds.Max.y, localBounds.Max.z },
				};

				for (const glm::vec3& corner : corners)
					bounds.Expand(glm::vec3(model * glm::vec4(corner, 1.0f)));
			}

			if (!bounds.IsValid())
			{
				bounds.Min = glm::vec3(-10.0f);
				bounds.Max = glm::vec3(10.0f);
			}

			return bounds;
		}
	}

	LightRenderData ExtractDirectionalLight(entt::registry& registry, uint32_t shadowMapSize)
	{
		const Bounds casterBounds = ComputeCasterBounds(registry);
		const glm::vec3 center = (casterBounds.Min + casterBounds.Max) * 0.5f;

		auto lightView = registry.view<LightComponent, TransformComponent>();
		for (auto entity : lightView)
		{
			const auto& [lightComponent, transformComponent] = lightView.get<LightComponent, TransformComponent>(entity);
			if (lightComponent.Type != LightType::Directional || lightComponent.Intensity <= 0.0f)
				continue;

			glm::vec3 direction = ComputeDown(transformComponent);

			const glm::vec3 extents = glm::max((casterBounds.Max - casterBounds.Min) * 0.5f, glm::vec3(5.0f));
			const float distance = glm::length(extents) + 25.0f;
			const glm::vec3 lightPosition = center - direction * distance;
			glm::vec3 up = glm::abs(direction.y) > 0.99f ? glm::vec3(0.f, 0.f, 1.f) : glm::vec3(0.f, 1.f, 0.f);

			LightRenderData result;
			result.Direction = direction;
			result.LightEntityID = static_cast<int>(entt::to_integral(entity));
			result.TexelSize = 1.0f / static_cast<float>(shadowMapSize);
			result.LightView = glm::lookAt(lightPosition, center, up);

			glm::vec3 lightSpaceMin{ FLT_MAX, FLT_MAX, FLT_MAX };
			glm::vec3 lightSpaceMax{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
			const glm::vec3 corners[8] = {
				{ casterBounds.Min.x, casterBounds.Min.y, casterBounds.Min.z },
				{ casterBounds.Max.x, casterBounds.Min.y, casterBounds.Min.z },
				{ casterBounds.Min.x, casterBounds.Max.y, casterBounds.Min.z },
				{ casterBounds.Max.x, casterBounds.Max.y, casterBounds.Min.z },
				{ casterBounds.Min.x, casterBounds.Min.y, casterBounds.Max.z },
				{ casterBounds.Max.x, casterBounds.Min.y, casterBounds.Max.z },
				{ casterBounds.Min.x, casterBounds.Max.y, casterBounds.Max.z },
				{ casterBounds.Max.x, casterBounds.Max.y, casterBounds.Max.z },
			};

			for (const glm::vec3& corner : corners)
			{
				const glm::vec3 lightSpace = glm::vec3(result.LightView * glm::vec4(corner, 1.0f));
				lightSpaceMin = glm::min(lightSpaceMin, lightSpace);
				lightSpaceMax = glm::max(lightSpaceMax, lightSpace);
			}

			const float xyPadding = 2.0f;
			const float zPadding = 20.0f;
			result.LightProjection = glm::ortho(
				lightSpaceMin.x - xyPadding,
				lightSpaceMax.x + xyPadding,
				lightSpaceMin.y - xyPadding,
				lightSpaceMax.y + xyPadding,
				-lightSpaceMax.z - zPadding,
				-lightSpaceMin.z + zPadding);
			result.LightViewProjection = result.LightProjection * result.LightView;
			result.Valid = true;
			return result;
		}

		return {};
	}

	void Render(entt::registry& registry, ShadowPass& shadowPass, const LightRenderData& lightData)
	{
		if (!lightData.Valid)
			return;

		auto view = registry.view<MeshComponent, TransformComponent>();
		for (auto entity : view)
		{
			if (!ShouldCastShadow(registry, entity))
				continue;

			const auto& [meshComponent, transformComponent] = view.get<MeshComponent, TransformComponent>(entity);
			if (!meshComponent.HasMesh())
				continue;

			shadowPass.SubmitMesh(meshComponent.MeshRef, transformComponent.GetTransform(), lightData.LightViewProjection);
		}
	}
}
