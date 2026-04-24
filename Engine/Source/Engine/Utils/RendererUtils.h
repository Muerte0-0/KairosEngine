#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Engine/Debugging/Log.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Vertex — Matches the final Mesh.slang input layout exactly.
	// -----------------------------------------------------------------------
	struct Vertex
	{
		glm::vec3 Position  { 0.f, 0.f, 0.f };
		glm::vec3 Normal    { 0.f, 1.f, 0.f };
		glm::vec3 Tangent   { 1.f, 0.f, 0.f };
		glm::vec3 Bitangent { 0.f, 0.f, 1.f };
		glm::vec2 TexCoord  { 0.f, 0.f };

		// Binding / attribute descriptions are generated from this layout.
		// Do not change member order without updating the pipeline's vertex input state.
	};

	// -----------------------------------------------------------------------
	// GpuLight — GPU-side light struct.
	// std140-compatible: every member 16-byte aligned.
	//
	// Type encoding (matches Mesh.slang):
	//   0 = Directional   — uses Direction, Color, Intensity
	//   1 = Point         — uses Position, Color, Intensity, Range
	//   2 = Spot          — uses Position, Direction, Color, Intensity, Range, cone angles
	// -----------------------------------------------------------------------
	struct alignas(16) GpuLight
	{
		glm::vec4 Position;         // xyz = world pos,       w = unused
		glm::vec4 Direction;        // xyz = normalized dir,  w = unused
		glm::vec4 ColorIntensity;   // xyz = linear color,    w = intensity
		float     Range;            // Point / Spot attenuation radius
		float     InnerConeAngle;   // Spot — cosine of inner half-angle
		float     OuterConeAngle;   // Spot — cosine of outer half-angle
		int       Type;             // 0=Directional 1=Point 2=Spot
	};

	static_assert(sizeof(GpuLight) % 16 == 0, "GpuLight must be 16-byte aligned for std140");

	// -----------------------------------------------------------------------
	// SceneData — Set 0 Binding 0 UBO.  Replaces old UniformBufferObject.
	// -----------------------------------------------------------------------
	static constexpr int MAX_LIGHTS = 16;

	struct SceneData
	{
		alignas(16) glm::mat4 View;
		alignas(16) glm::mat4 Proj;
		alignas(16) glm::mat4 LightViewProj;
		alignas(16) GpuLight  Lights[MAX_LIGHTS];
		alignas(16) glm::vec4 ShadowParams{ 0.f, 0.f, 0.f, 0.f };
		alignas(4)  int       LightCount = 0;
		alignas(4)  int       ShadowLightIndex = -1;
		alignas(4)  int       ShadowEnabled = 0;
		float                 _pad0 = 0.f;
	};

	// Keep old name as alias so any remaining code still compiles during transition.
	using UniformBufferObject = SceneData;

	struct PushConstantObject
	{
		alignas(16) glm::mat4 Model;
	};
	
	enum class TextureFormat : uint8_t
	{
		Undefined = 0,

		// Color formats (Linear)
		RGBA8_UNorm,
		BGRA8_UNorm,

		// Color formats (sRGB)
		RGBA8_SRGB,
		BGRA8_SRGB,

		RGBA16_Float,
		RGBA32_Float,
		 
		// Depth formats
		Depth24Stencil8,
		Depth32_Float,

		// Optional extras
		R8_UNorm,
		RG8_UNorm
	};
	
	enum class SampleCountBits : uint8_t
	{
		s1 = 1,
		s2 = 2,
		s4 = 4,
		s8 = 8,
		s16 = 16
	};
	
	enum class ShaderStage : uint8_t
	{
		Vertex = 0,
		Fragment,
		Compute
	};

	struct ShaderStageData
	{
		ShaderStage				Stage		= ShaderStage::Vertex;
		std::filesystem::path	Filepath;
		std::string				EntryPoint	= "main";
		std::vector<uint32_t>	Bytecode;
	};
	
	enum class ShaderDataType : uint8_t
	{
		None = 0,
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		Bool,
		Mat3, Mat4
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
			case ShaderDataType::Mat3:		return 4 * 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			case ShaderDataType::Bool:		return 1;
			default: break;
		}

		LOG(LogLevel::Error, "Unknown ShaderDataType!");
		return 0;
	}
}

namespace std
{
	template<>
	struct hash<Engine::ShaderStage>
	{
		size_t operator()(Engine::ShaderStage stage) const noexcept
		{
			return hash<int>{}(static_cast<int>(stage));
		}
	};
}
