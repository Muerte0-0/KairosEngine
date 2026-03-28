#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Engine
{
	enum class ShaderStage
	{
		Vertex = 0,
		Fragment,
		Compute
	};

	struct ShaderBinary
	{
		std::vector<uint32_t> Bytecode;
	};

	struct ShaderCreateInfo
	{
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
		const std::filesystem::path& GetFilepath() const { return m_CreateInfo.Filepath; }
		const std::string& GetEntryPoint() const { return m_CreateInfo.EntryPoint; }
		ShaderStage GetStage() const { return m_CreateInfo.Stage; }

		static const char* ShaderStageToString(ShaderStage stage);

	protected:
		ShaderCreateInfo m_CreateInfo;
	};
}
