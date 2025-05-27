#pragma once

#include "Engine/ImGui/ImGuiLayer.h"

namespace Kairos
{
	class VulkanImGuiLayer : public ImGuiLayer
	{
	public:
		VulkanImGuiLayer();
		~VulkanImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		virtual void Begin();
		virtual void End();
	private:
		float m_Time = 0.0f;
	};
}