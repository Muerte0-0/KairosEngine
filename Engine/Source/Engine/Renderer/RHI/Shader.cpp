#include "kepch.h"
#include "Shader.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
#	include "APIs/Vulkan/VulkanShader.h"
#endif

namespace Engine
{
	Shader::Shader(ShaderCreateInfo createInfo)
		: m_CreateInfo(std::move(createInfo))
	{}

	const char* Shader::ShaderStageToString(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:   return "Vertex";
		case ShaderStage::Fragment: return "Fragment";
		case ShaderStage::Compute:  return "Compute";
		default:                    return "Unknown";
		}
	}

	Scope<Shader> Shader::Create(ShaderCreateInfo createInfo, ShaderBinary binary)
	{
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
		// Both platforms currently use the Vulkan backend.
		// The device is retrieved from the active Renderer.
		return CreateScope<VulkanShader>(std::move(createInfo), std::move(binary));
#else
		KE_CORE_ASSERT(false, "Shader::Create — unsupported platform!");
		return nullptr;
#endif
	}
}
