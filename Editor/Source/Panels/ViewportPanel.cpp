#include "ViewportPanel.h"
#include "SceneHierarchyPanel.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>
#include "Engine/Math/Math.h"

namespace Kairos
{
	void ViewportPanel::Init(SceneRenderer* renderer, SceneCamera* camera,
	                         SceneCameraController* controller, CameraManager* cameraManager,
	                         SceneHierarchyPanel* hierarchy)
	{
		m_Renderer      = renderer;
		m_Camera        = camera;
		m_Controller    = controller;
		m_CameraManager = cameraManager;
		m_Hierarchy     = hierarchy;
		m_GizmoType     = ImGuizmo::OPERATION::TRANSLATE;
	}

	void ViewportPanel::OnImGuiRender()
	{
		ImGui::Begin("Viewport");

		ImVec2 size = ImGui::GetContentRegionAvail();
		m_Size = { size.x, size.y };

		Engine::Framebuffer* fb = m_Renderer ? m_Renderer->GetFramebuffer() : nullptr;
		if (fb && size.x > 0.f && size.y > 0.f)
		{
			m_Renderer->Resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
			m_Camera->OnViewportResize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));

			if (void* id = fb->GetImGuiTextureID())
			{
				ImGui::Image(id, size, ImVec2(0, 1), ImVec2(1, 0));

				ImVec2 mn = ImGui::GetItemRectMin();
				ImVec2 mx = ImGui::GetItemRectMax();
				m_Bounds[0] = { mn.x, mn.y };
				m_Bounds[1] = { mx.x, mx.y };
			}
		}
		else
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Implemented Yet! :)");
		}

		// Drag-drop
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("SCENE_ITEM"))
			{
				if (OnSceneDrop)
					OnSceneDrop(std::filesystem::path(static_cast<const wchar_t*>(p->Data)));
			}
			if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("MESH_ITEM"))
			{
				if (OnMeshDrop)
					OnMeshDrop(std::filesystem::path(static_cast<const wchar_t*>(p->Data)));
			}
			ImGui::EndDragDropTarget();
		}

		m_Focused = ImGui::IsWindowFocused();
		m_Hovered = ImGui::IsWindowHovered();

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && m_Hovered && !m_Focused)
		{
			ImGui::SetWindowFocus();
			m_Focused = true;
		}

		DrawGizmos();

		ImGui::End();
	}

	void ViewportPanel::DrawGizmos()
	{
		if (!m_Hierarchy) return;
		Entity selected = m_Hierarchy->GetSelectedEntity();
		if (!selected || m_GizmoType == -1) return;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
		                  ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		auto& tc = selected.GetComponent<TransformComponent>();
		glm::mat4 transform = tc.GetTransform();

		bool snap = Engine::Input::IsKeyPressed(Engine::KeyBoard::LeftControl);
		float snapValue = (m_GizmoType == ImGuizmo::OPERATION::ROTATE) ? 5.0f : 0.5f;
		float snapValues[3] = { snapValue, snapValue, snapValue };

		ImGuizmo::Manipulate(
			glm::value_ptr(m_CameraManager->GetActiveCamera()->GetView()),
			glm::value_ptr(m_CameraManager->GetActiveCamera()->GetProjection()),
			static_cast<ImGuizmo::OPERATION>(m_GizmoType), ImGuizmo::LOCAL,
			glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

		if (ImGuizmo::IsUsing())
		{
			glm::vec3 translation, rotation, scale;
			Engine::Math::DecomposeTransform(transform, translation, rotation, scale);
			glm::vec3 deltaRotation = rotation - tc.Rotation;
			tc.Translation  = translation;
			tc.Rotation    += deltaRotation;
			tc.Scale        = scale;
		}
	}
}
