#pragma once
#include "Engine/Renderer/Shader.h"

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

namespace Kairos
{
	class PipelineLayoutBuilder
	{
	public:
		PipelineLayoutBuilder(VkDevice& logicalDevice);

		VkPipelineLayout Build(std::deque<std::function<void(VkDevice)>>& deletionQueue);

		inline void Add(VkDescriptorSetLayout descriptorSetLayout) { m_DescriptorLayouts.push_back(descriptorSetLayout); }
	private:
		VkDevice& m_LogicalDevice;
		std::vector<VkDescriptorSetLayout> m_DescriptorLayouts;

		void Reset();
	};

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& filepath);
		VulkanShader(const std::string& vertFilepath, const std::string& fragFilepath);
		VulkanShader(std::string& shaderName, std::string& vertCode, std::string& fragCode);

		virtual void Bind() const override {};
		virtual void UnBind() const override {};

		virtual const std::string& GetName() const override { return m_ShaderName; }
		const std::vector<VkShaderEXT>& GetShaders() const { return m_Shaders; }
		const VkDescriptorSet& GetDescriptorSet() const { return m_DescriptorSet; }

		std::vector<VkShaderEXT> MakeShaderObjects(VkDevice device, const char* name, std::vector<char> vertSrc, std::vector<char> fragSrc,
			std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue, bool compileCode);

		void UploadUniformInt(const std::string& name, int value) override;

		void UploadUniformFloat(const std::string& name, float value) override;
		void UploadUniformVec2(const std::string& name, const glm::vec2& values) override;
		void UploadUniformVec3(const std::string& name, const glm::vec3& values) override;
		void UploadUniformVec4(const std::string& name, const glm::vec4& values) override;

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) override;
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;

		void UploadSceneData(const std::string& name, const SceneData& sceneData) override;

	private:
		std::vector<char> ReadCode(const std::string& filepath);
		std::string ReadFile(const std::string& filepath);

		std::unordered_map<shaderc_shader_kind, std::string> PreProcess(const std::string& source);
		void PreProcessShader(std::unordered_map<shaderc_shader_kind, std::vector<char>>& shaderSources, shaderc::CompileOptions& options);
		void CompileToAssembly(std::unordered_map<shaderc_shader_kind, std::vector<char>>& shaderSources, shaderc::CompileOptions& options);
		std::vector<uint32_t> Compile(std::vector<char>& shaderSource, shaderc::CompileOptions& options);

		void CreateDescriptorSet();

		std::string m_ShaderName;

		std::vector<VkShaderEXT> m_Shaders;

		VkBuffer m_SceneDataBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_SceneDataMemory = VK_NULL_HANDLE;
		VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
	};
}