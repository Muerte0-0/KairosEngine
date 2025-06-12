#pragma once
#include "Engine/Renderer/Shader.h"
#include <glm/glm.hpp>

#include <glad/gl.h>

namespace Kairos
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& vertFilePath, const std::string& fragFilePath);
		OpenGLShader(std::string& shaderName, std::string& vertCode, std::string& fragCode);
		~OpenGLShader();

		void Bind() const override;
		void UnBind() const override;
		virtual const std::string& GetName() const override { return m_Name; }

		void UploadUniformInt(const std::string& name, int value) override;

		void UploadUniformFloat(const std::string& name, float value) override;
		void UploadUniformVec2(const std::string& name, const glm::vec2& values) override;
		void UploadUniformVec3(const std::string& name, const glm::vec3& values) override;
		void UploadUniformVec4(const std::string& name, const glm::vec4& values) override;

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) override;
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;

		void UploadSceneData(const uint32_t& bindingIndex, const SceneData& sceneData) override;

	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
	private:
		uint32_t m_RendererID = 0;
		std::string m_Name;

		GLuint m_SceneDataUBO;
	};
}