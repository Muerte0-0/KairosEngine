#include "kepch.h"
#include "Shader.h"

#include "APIs/Vulkan/VulkanShader.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Shader  — base
	// -----------------------------------------------------------------------

	Shader::Shader(const ShaderDescriptor& descriptor)
		: m_Name(descriptor.Name)
		, m_ShaderDirectory(descriptor.ShaderDirectory)
	{
		for (const ShaderStageData& stage : descriptor.Stages)
			m_Stages[stage.Stage] = stage;
	}

	bool Shader::HasStage(ShaderStage stage) const
	{
		return m_Stages.contains(stage);
	}

	const ShaderStageData& Shader::GetStage(ShaderStage stage) const
	{
		ASSERT(HasStage(stage), "Shader '{}': requested stage '{}' does not exist.",
			m_Name, StageToString(stage));
		return m_Stages.at(stage);
	}

	std::vector<const ShaderStageData*> Shader::GetStages() const
	{
		std::vector<const ShaderStageData*> result;
		result.reserve(m_Stages.size());
		for (const auto& data : m_Stages | views::values)
			result.push_back(&data);
		return result;
	}

	const char* Shader::StageToString(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:   return "Vertex";
		case ShaderStage::Fragment: return "Fragment";
		case ShaderStage::Compute:  return "Compute";
		default:                    return "Unknown";
		}
	}

	Ref<Shader> Shader::Create(const ShaderDescriptor& descriptor)
	{
		ASSERT(!descriptor.Name.empty(),      "Shader::Create — descriptor has no name.");
		ASSERT(!descriptor.Stages.empty(),    "Shader::Create — descriptor has no stages.");
		
		Ref<Shader> result = nullptr;
		
		switch (Renderer::GetAPI()->GetType())
		{
		case API::Vulkan:
			result = CreateRef<VulkanShader>(descriptor);
			break;
#ifdef PLATFORM_WINDOWS
		case API::DX11:
			ASSERT(false, "API[Direct X 11]: Not Implemented");
			break;
		case API::DX12:
			ASSERT(false, "API[Direct X 12]: Not Implemented");
			break;
#endif
		}
		
		return result;
	}

	// -----------------------------------------------------------------------
	// Shader Library
	// -----------------------------------------------------------------------

	Ref<Shader> ShaderLibrary::Load(const ShaderDescriptor& descriptor)
	{
		if (Exists(descriptor.Name))
		{
			LOG(LogLevel::Warning, "ShaderLibrary: '{}' is already loaded — returning cached shader.", descriptor.Name);
			return m_Shaders.at(descriptor.Name);
		}

		Ref<Shader> shader = Shader::Create(descriptor);
		shader->Init();
		m_Shaders[descriptor.Name] = shader;

		LOG(LogLevel::Info, "ShaderLibrary: loaded '{}'.", descriptor.Name);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name) const
	{
		ASSERT(Exists(name), "ShaderLibrary: shader '{}' not found.", name);
		return m_Shaders.at(name);
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_Shaders.contains(name);
	}

	void ShaderLibrary::Clear()
	{
		for (auto& shader : m_Shaders | views::values)
			shader->Shutdown();
		
		m_Shaders.clear();
	}
}
