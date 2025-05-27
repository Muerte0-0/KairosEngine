#include "kepch.h"
#include "Texture.h"

#include "Renderer.h"

#include "API/OpenGL/OpenGLTexture.h"

namespace Kairos
{
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> Texture2D Not Implemented!"); return nullptr;
			case RenderAPI::API::OpenGL:			return std::make_shared<OpenGLTexture2D>(path);
			case RenderAPI::API::Vulkan:			KE_CORE_ASSERT(false, "RendererAPI::Vulkan -> Texture2D Not Implemented!"); return nullptr;
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> Texture2D Not Implemented!"); return nullptr;
			#endif // KE_PLATFORM_WINDOWS
		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}