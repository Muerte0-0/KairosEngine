#pragma once

#include "GraphicsContext.h"

namespace Kairos
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual const std::string& GetName() const = 0;

		// Shader Code From Single File
		static Ref<Shader> Create(const std::string& filepath);

		// Shader Code From File
		static Ref<Shader> Create(const std::string& vertexFilepth, const std::string& fragmentFilepth);

		// Shader Code
		static Ref<Shader> Create(std::string& shaderName, std::string& vertCode, std::string& fragCode);
	};

	class ShaderLibrary
	{
	public:
		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);

		std::unordered_map<std::string, Ref<Shader>> GetShaders() { return m_Shaders; }

		Ref<Shader> Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}