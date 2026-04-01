#pragma once

#include "Engine/Renderer/RendererUtils.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Engine
{
	// -----------------------------------------------------------------------
	// ShaderBinary
	// -----------------------------------------------------------------------

	struct ShaderBinary
	{
		std::vector<uint32_t> Bytecode;
	};

	// -----------------------------------------------------------------------
	// ShaderCreateInfo
	// -----------------------------------------------------------------------

	struct ShaderCreateInfo
	{
		std::string           Name;
		std::filesystem::path Filepath;
		std::string           EntryPoint = "main";
		ShaderStage           Stage      = ShaderStage::Vertex;
	};

	// -----------------------------------------------------------------------
	// Shader — abstract RHI interface
	// -----------------------------------------------------------------------

	class Shader
	{
	public:
		explicit Shader(ShaderCreateInfo createInfo);
		virtual ~Shader() = default;

		// No copying — GPU resources are move-only.
		Shader(const Shader&)            = delete;
		Shader& operator=(const Shader&) = delete;

		[[nodiscard]] const ShaderCreateInfo&   GetCreateInfo()  const { return m_CreateInfo; }
		[[nodiscard]] const std::string&        GetName()        const { return m_CreateInfo.Name; }
		[[nodiscard]] const std::filesystem::path& GetFilepath() const { return m_CreateInfo.Filepath; }
		[[nodiscard]] const std::string&        GetEntryPoint()  const { return m_CreateInfo.EntryPoint; }
		[[nodiscard]] ShaderStage               GetStage()       const { return m_CreateInfo.Stage; }

		static const char* ShaderStageToString(ShaderStage stage);

		/**
		 * @brief Factory — dispatches to the active RHI backend.
		 * @param createInfo  Shader metadata (name, path, entry, stage).
		 * @param binary      Pre-loaded SPIR-V / bytecode.
		 * @return Owning pointer to the concrete Shader implementation.
		 */
		[[nodiscard]] static Scope<Shader> Create(ShaderCreateInfo createInfo, ShaderBinary binary);

	protected:
		ShaderCreateInfo m_CreateInfo;
	};
}
