#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"

#include <string>
#include <vector>

namespace Engine
{
	// Key strings used for serialization — treat as stable identifiers.
	namespace PrimitiveKey
	{
		inline constexpr const char* Cube   = "Engine://Primitives/Cube";
		inline constexpr const char* Plane  = "Engine://Primitives/Plane";
		inline constexpr const char* Sphere = "Engine://Primitives/Sphere";
	}

	struct PrimitiveEntry
	{
		const char* DisplayName; // "Cube", "Plane", "Sphere"
		const char* Key;         // PrimitiveKey::* — stable serialization ID
	};

	class PrimitiveMeshFactory
	{
	public:
		// Mesh generators (always create new Mesh)
		static Ref<Mesh> CreateCube(float h = 1);
		static Ref<Mesh> CreatePlane(float h = 1, uint32_t sub = 1);
		static Ref<Mesh> CreateSphere(float radius = 1, uint32_t rings = 16, uint32_t sectors = 16);

		// Cached lookup — returns same Ref for same key, creates on first call.
		// Returns nullptr for unknown key.
		static Ref<Mesh> GetOrCreate(const std::string& key);

		// Release all cached meshes. Must be called before Renderer::Shutdown()
		// to ensure GPU buffers are freed before the Vulkan device is destroyed.
		static void Shutdown();

		// All registered primitives — used by editor UI.
		static const std::vector<PrimitiveEntry>& GetRegistry();
	};
}
