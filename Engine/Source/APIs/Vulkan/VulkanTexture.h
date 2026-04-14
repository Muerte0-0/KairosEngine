#pragma once
#include "Engine/Renderer/RHI/Resources/Texture.h"
#include <vulkan/vulkan_raii.hpp>

namespace Engine {

	class VulkanTexture final : public Texture
	{
	public:
		explicit VulkanTexture(const std::filesystem::path& path, const TextureSpecification& spec);
		VulkanTexture(const void* data, uint32_t size, const TextureSpecification& spec);
		~VulkanTexture() override;

		uint32_t		GetWidth()		const override { return m_Spec.Width;	}
		uint32_t		GetHeight()		const override { return m_Spec.Height;	}
		TextureFormat	GetFormat()		const override { return m_Spec.Format;	}
		bool			IsLoaded()		const override { return m_Loaded;		}

		// Lazily registers with ImGui_ImplVulkan_AddTexture on first call.
		// Returns the VkDescriptorSet cast to uint64_t. Cast to ImTextureID at call site.
		uint64_t		GetTextureID()	const override;

		// Raw Vulkan accessors for descriptor set wiring
		vk::Image     GetImage()     const { return *m_Image;     }
		vk::ImageView GetImageView() const { return *m_ImageView; }
		vk::Sampler   GetSampler()   const { return *m_Sampler;   }

	private:
		void UploadToGPU(const void* pixelData, uint32_t dataSize);
		void CreateSampler();
		void ReleaseTextureID();

		TextureSpecification    m_Spec;
		bool                    m_Loaded       { false };

		vk::raii::Image         m_Image        { nullptr };
		vk::raii::DeviceMemory  m_ImageMemory  { nullptr };
		vk::raii::ImageView     m_ImageView    { nullptr };
		vk::raii::Sampler       m_Sampler      { nullptr };

		mutable uint64_t        m_TextureID    { 0 }; // cached VkDescriptorSet for ImGui
	};

}
