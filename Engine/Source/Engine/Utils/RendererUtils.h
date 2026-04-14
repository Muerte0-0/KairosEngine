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

		// Binding / attribute descriptions are generated from this layout
		// do not change member order without updating the pipeline's vertex input state.
	};

	struct UniformBufferObject
	{
		alignas(16) glm::mat4 View;
		alignas(16) glm::mat4 Proj;
	};

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

		// Optional extras (nice to have)
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
	/**
		* @brief All data describing a single shader stage.
		*
		* Passed into Shader::Create() as part of a Shader Descriptor.
		* The Bytecode field starts empty and is populated later when shader is Created
	*/
	struct ShaderStageData
	{
		ShaderStage				Stage		= ShaderStage::Vertex;
		std::filesystem::path	Filepath;								// Relative to Shader Directory.
		std::string				EntryPoint	= "main";
		std::vector<uint32_t>	Bytecode;								// Filled during Init().
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

// std::unordered_map<ShaderStage, ...> requires a hash specialization.
template<>
struct std::hash<Engine::ShaderStage>
{
	size_t operator()(Engine::ShaderStage stage) const noexcept
	{
		return std::hash<int>{}(static_cast<int>(stage));
	}
};
