#pragma once

#include <cstdint>

namespace Engine
{
	struct FramebufferSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
	};

	class Framebuffer
	{
	public:
		explicit Framebuffer(FramebufferSpecification specification);
		virtual ~Framebuffer() = default;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void* GetImGuiTextureID() = 0;

		const FramebufferSpecification& GetSpecification() const { return m_Specification; }
		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }

	protected:
		FramebufferSpecification m_Specification;
	};
}
