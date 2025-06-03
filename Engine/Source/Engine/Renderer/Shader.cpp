#include "kepch.h"
#include "Shader.h"

#include <fstream>
#include "Renderer.h"

#include "API/OpenGL/OpenGLShader.h"
#include "API/Vulkan/VulkanShader.h"

namespace Kairos
{
	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> Shaders Not Implemented!"); return nullptr;
			case RenderAPI::API::OpenGL:			return CreateRef<OpenGLShader>(filepath);
			case RenderAPI::API::Vulkan:			return CreateRef<VulkanShader>(filepath);
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> Shaders Not Implemented!"); return nullptr;
			#endif // KE_PLATFORM_WINDOWS
		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& vertFilepath, const std::string& fragFilepath)
	{
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> Shaders Not Implemented!");
			case RenderAPI::API::OpenGL:			return CreateRef<OpenGLShader>(vertFilepath, fragFilepath);
			case RenderAPI::API::Vulkan:			return CreateRef<VulkanShader>(vertFilepath, fragFilepath);
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> Shaders Not Implemented!");
			#endif // KE_PLATFORM_WINDOWS
		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Shader> Shader::Create(std::string& shaderName, std::string& vertCode, std::string& fragCode)
	{
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> Shaders Not Implemented!");
			case RenderAPI::API::OpenGL:			return CreateRef<OpenGLShader>(shaderName, vertCode, fragCode);
			case RenderAPI::API::Vulkan:			return CreateRef<VulkanShader>(shaderName, vertCode, fragCode);
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> Shaders Not Implemented!");
			#endif // KE_PLATFORM_WINDOWS
		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{
		KE_CORE_ASSERT(!Exists(name), "Shader Already Exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		KE_CORE_ASSERT(Exists(name), "Shader Not Found!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}
}