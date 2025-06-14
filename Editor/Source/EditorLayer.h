#pragma once
#include "Engine.h"

namespace Kairos
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
	};

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;
	private:
		Ref<VertexArray> m_VertexArray;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];

		PerspectiveCamera m_PerspectiveCamera;

		float NumFrames = 0;
	};
}