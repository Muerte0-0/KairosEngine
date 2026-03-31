#pragma once

#include "Engine/Renderer/RendererUtils.h"

namespace Engine
{
	struct ShaderBinary
	{
		std::vector<uint32_t> Bytecode;
	};

	struct ShaderCreateInfo
	{
		std::string Name;
		std::filesystem::path Filepath;
		std::string EntryPoint = "main";
		ShaderStage Stage = ShaderStage::Vertex;
	};

	class Shader
	{
	public:
		explicit Shader(ShaderCreateInfo createInfo);
		virtual ~Shader() = default;

		const ShaderCreateInfo& GetCreateInfo() const { return m_CreateInfo; }
		const std::string& GetName() const { return m_CreateInfo.Name; }
		const std::filesystem::path& GetFilepath() const { return m_CreateInfo.Filepath; }
		const std::string& GetEntryPoint() const { return m_CreateInfo.EntryPoint; }
		ShaderStage GetStage() const { return m_CreateInfo.Stage; }

		static const char* ShaderStageToString(ShaderStage stage);

	protected:
		ShaderCreateInfo m_CreateInfo;
	};
}
