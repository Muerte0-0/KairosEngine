#pragma once

#include "Engine/Utils/RendererUtils.h"
#include "Engine/Core/Base.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine
{
	// -----------------------------------------------------------------------
	// Shader Descriptor
	// -----------------------------------------------------------------------

	struct ShaderDescriptor
	{
		std::string						Name;
		std::filesystem::path			ShaderDirectory;
		std::vector<ShaderStageData>	Stages;				// One entry per stage to compile.
	};

	// -----------------------------------------------------------------------
	// Shader
	// -----------------------------------------------------------------------

	class Shader
	{
	public:
		virtual ~Shader() = default;

		Shader(const Shader&)            = delete;
		Shader& operator=(const Shader&) = delete;

		// ------------------------------------------------------------------
		// Accessors
		// ------------------------------------------------------------------
		const std::string&         GetName()   const { return m_Name; }
		bool                        HasStage(ShaderStage stage)  const;
		const ShaderStageData&      GetStage(ShaderStage stage)  const;
		std::vector<const  ShaderStageData*> GetStages() const;
		
		/**
		 * @brief Initialize all GPU resources for every stage.
		 *        Must be called once after Create() before the shader is used.
		*/
		virtual void Init() = 0;

		/**
		 * @brief Release all GPU resources. Safe to call multiple times.
		*/
		virtual void Shutdown() = 0;
		
		[[nodiscard]] static Ref<Shader> Create(const ShaderDescriptor& descriptor);

		// ------------------------------------------------------------------
		// Utilities
		// ------------------------------------------------------------------
		static const char* StageToString(ShaderStage stage);

	protected:
		explicit Shader(const ShaderDescriptor& descriptor);

		std::string                                         m_Name;
		std::filesystem::path                               m_ShaderDirectory;
		std::unordered_map<ShaderStage, ShaderStageData>    m_Stages;
	};

	// -----------------------------------------------------------------------
	// Shader Library
	// -----------------------------------------------------------------------

	class ShaderLibrary
	{
	public:
		/**
		 * @brief Load a shader from a descriptor, add it to the library, and
		 *        return it. If a shader with the same name already exists the
		 *        cached version is returned without reloading.
		*/
		[[nodiscard]] Ref<Shader> Load(const ShaderDescriptor& descriptor);

		/**
		 * @brief Retrieve an already-loaded shader by name.
		 *        Asserts if the name is not found.
		*/
		[[nodiscard]] Ref<Shader> Get(const std::string& name) const;

		[[nodiscard]] bool Exists(const std::string& name) const;

		void Clear();

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}