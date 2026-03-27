#pragma once
#include "Engine/ImGui/ImGuiLayer.h"

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	class VulkanImGuiLayer : public ImGuiLayer
	{
	public:
		VulkanImGuiLayer();
		~VulkanImGuiLayer() override = default;
        
		void OnAttach() override;
		void OnDetach() override;
        
		void Begin() override;
		void End() override;
        
		void OnImGuiRender() override;
        
	private:
		void InitializeVulkanForImGui() const;
		
		vk::raii::DescriptorPool m_ImGuiPool = nullptr;
	};
}
