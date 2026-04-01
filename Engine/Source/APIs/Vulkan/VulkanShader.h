#pragma once

#include "Engine/Renderer/RHI/Shader.h"

#include <vulkan/vulkan_raii.hpp>
#include <unordered_map>

namespace Engine
{
	// -----------------------------------------------------------------------
	// Vulkan Shader
	// -----------------------------------------------------------------------

	class VulkanShader final : public Shader
	{
	public:
		explicit VulkanShader(const ShaderDescriptor& descriptor);
		~VulkanShader() override;
		/**
		 * @brief Load SPIR-V from disk and create all VkShaderModules.
		 *        Calls Shutdown() first, so it is safe to call again after
		 *        a hot-reload.
		 */
		void Init()     override;
		void Shutdown() override;

		// ------------------------------------------------------------------
		// Vulkan-specific accessors
		// ------------------------------------------------------------------

		/** @return The VkShaderModule for the given stage. Asserts if missing. */
		[[nodiscard]] const vk::raii::ShaderModule& GetModule(ShaderStage stage) const;

		/** @return vk::ShaderStageFlagBits mapped from a ShaderStage. */
		[[nodiscard]] static vk::ShaderStageFlagBits ToVkStage(ShaderStage stage);

	private:
		std::unordered_map<ShaderStage, vk::raii::ShaderModule> m_Modules;

		/** Read a SPIR-V file from disk into a ShaderStageData::Spirv vector. */
		void LoadSpirvFromDisk(ShaderStageData& stageData) const;

		/** Create a VkShaderModule from an already-populated Spirv vector. */
		void CreateModule(ShaderStageData& stageData);
	};
}
