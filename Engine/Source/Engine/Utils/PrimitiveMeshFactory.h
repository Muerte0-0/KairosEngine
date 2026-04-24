#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"

namespace Engine
{
	class PrimitiveMeshFactory
	{
	public:
		static Ref<Mesh> CreateCube(float h = 1);
		static Ref<Mesh> CreatePlane(float h = 1, uint32_t sub = 1);
		static Ref<Mesh> CreateSphere(float radius = 1, uint32_t rings = 8, uint32_t sectors = 8);
	};
}
