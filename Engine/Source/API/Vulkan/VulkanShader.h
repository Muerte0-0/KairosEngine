#pragma once
#include "Engine/Renderer/Shader.h"

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
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

	private:
		std::vector<char> ReadCode(const std::string& filepath);
		std::string ReadFile(const std::string& filepath);
		std::string m_ShaderName;
	};
}