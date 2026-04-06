#pragma once

#include "Engine/Utils/RendererUtils.h"

namespace Engine
{
	struct FramebufferSpecification
	{
		uint32_t Width  = 1;
		uint32_t Height = 1;

		TextureFormat   ColorFormat = TextureFormat::Undefined;
		TextureFormat   DepthFormat = TextureFormat::Undefined;
		SampleCountBits SampleCount = SampleCountBits::s1;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void* GetImGuiTextureID() = 0;
		
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		
		[[nodiscard]] static Scope<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}
