#include "kepch.h"
#include "VulkanShader.h"

#include "Engine/Core/Application.h"
#include "VulkanContext.h"


namespace Kairos
{
	VulkanShader::VulkanShader(const std::string& filepath)
	{
		
	}

	VulkanShader::VulkanShader(const std::string& vertFilepath, const std::string& fragFilepath)
	{
		
	}

	VulkanShader::VulkanShader(std::string& shaderName, std::string& vertCode, std::string& fragCode) : m_ShaderName(shaderName)
	{
		
	}

	void VulkanShader::Bind() const
	{

	}

	void VulkanShader::UnBind() const
	{

	}

	std::vector<char> VulkanShader::ReadCode(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			KE_CORE_ERROR("Failed To Open File!");

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	std::string VulkanShader::ReadFile(const std::string& filepath)
	{
		std::string result;

		std::ifstream in(filepath, std::ios::in | std::ios::binary);

		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
			KE_CORE_ERROR("Could not Open File '{0}'", filepath);

		return result;
	}
}