#include "kepch.h"
#include "GameCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Engine
{
	void GameCamera::SyncFromEntityTransform(const glm::mat4& worldTransform)
	{
		// View = inverse of the camera's world transform.
		// glm::inverse is fine for TRS matrices; no need for full LookAt rebuild.
		SetView(glm::inverse(worldTransform));
	}

	void GameCamera::SetViewFromTransform(const glm::vec3& position, const glm::vec3& eulerRadians)
	{
		glm::mat4 rot      = glm::toMat4(glm::quat(eulerRadians));
		glm::mat4 worldMat = glm::translate(glm::mat4(1.0f), position) * rot;
		SetView(glm::inverse(worldMat));
	}
}
