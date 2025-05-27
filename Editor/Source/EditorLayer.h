#pragma once
#include "Engine.h"

namespace Kairos
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep delta) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;
	private:
		Ref<VertexArray> m_VertexArray;
		Ref<Shader> m_TriangleShader;

		float NumFrames = 0;
	};
}