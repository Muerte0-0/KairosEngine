#pragma once
#include "Engine/Renderer/Shader.h"


#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

namespace Kairos
{
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& filepath);
		VulkanShader(const std::string& vertFilepath, const std::string& fragFilepath);
		VulkanShader(std::string& shaderName, std::string& vertCode, std::string& fragCode);

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual const std::string& GetName() const override { return m_ShaderName; }
		const std::vector<VkShaderEXT>& GetShaders() const { return m_Shaders; }

		std::vector<VkShaderEXT> MakeShaderObjects(VkDevice device, const char* name, std::vector<char> vertSrc, std::vector<char> fragSrc,
			std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue, bool compileCode);
	private:
		std::vector<char> ReadCode(const std::string& filepath);
		std::string ReadFile(const std::string& filepath);
		std::string m_ShaderName;

		std::vector<VkShaderEXT> m_Shaders;

		std::unordered_map<shaderc_shader_kind, std::string> PreProcess(const std::string& source);
		void PreProcessShader(std::unordered_map<shaderc_shader_kind, std::vector<char>>& shaderSources, shaderc::CompileOptions& options);
		void CompileToAssembly(std::unordered_map<shaderc_shader_kind, std::vector<char>>& shaderSources, shaderc::CompileOptions& options);
		std::vector<uint32_t> Compile(std::vector<char>& shaderSource, shaderc::CompileOptions& options);
	};
}