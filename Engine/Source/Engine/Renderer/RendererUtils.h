#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace Engine
{
	enum class TextureFormat
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
	
	enum class SampleCountBits
	{
		s1 = 1,
		s2 = 2,
		s4 = 4,
		s8 = 8,
		s16 = 16
	};
	
	enum class ShaderStage
	{
		Vertex = 0,
		Fragment,
		Compute
	};
	/**
		* @brief All data describing a single shader stage.
		*
		* Passed into Shader::Create() as part of a Shader Descriptor.
		* The Spirv field starts empty and is populated by VulkanShader::Init()
		* when it loads the compiled SPIR-V binary from disk.
	*/
	struct ShaderStageData
	{
		ShaderStage           Stage      = ShaderStage::Vertex;
		std::filesystem::path Filepath;              // Relative to Shader Directory.
		std::string           EntryPoint = "main";
		std::vector<uint32_t> Spirv;                 // Filled during Init().
	};
	
	enum class ShaderDataType
	{
		None = 0,
		Float, Float2, Float3, Float4,
		Mat3, Mat4
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:  return 4;
			case ShaderDataType::Float2: return 4 * 2;
			case ShaderDataType::Float3: return 4 * 3;
			case ShaderDataType::Float4: return 4 * 4;
			case ShaderDataType::Mat3:   return 4 * 3 * 3;
			case ShaderDataType::Mat4:   return 4 * 4 * 4;
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