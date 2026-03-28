#include "kepch.h"
#include "Shader.h"

namespace Engine
{
	Shader::Shader(ShaderCreateInfo createInfo)
		: m_CreateInfo(std::move(createInfo))
	{
	}

	const char* Shader::ShaderStageToString(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:
			return "Vertex";
		case ShaderStage::Fragment:
			return "Fragment";
		case ShaderStage::Compute:
			return "Compute";
		default:
			return "Unknown";
		}
	}
}
