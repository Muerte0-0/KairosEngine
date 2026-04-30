#include "ViewportPanel.h"
#include "SceneHierarchyPanel.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Engine/Math/Math.h"
#include "Engine/Scene/SceneGraph.h"

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

	// -----------------------------------------------------------------------
	// Toolbar — drawn as an overlay inside the viewport window
	// -----------------------------------------------------------------------
	static void DrawToolbar(int& gizmoType, int& gizmoMode)
	{
		const float PAD        = 8.f;
		const float BTN_SIZE   = 28.f;
		const float BTN_ROUND  = 4.f;
		const float SEPARATOR  = 6.f;

		ImVec2 winPos  = ImGui::GetWindowPos();
		ImVec2 winSize = ImGui::GetWindowSize();

		// Total width: 4 op buttons + separator + 2 mode buttons
		float totalW = 4 * (BTN_SIZE + PAD) - PAD + SEPARATOR + 2 * (BTN_SIZE + PAD) + PAD * 2;
		float startX = winPos.x + PAD * 2;
		float startY = winPos.y + PAD;

		// Background pill via DrawList
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddRectFilled({ startX, startY },
		                  { startX + totalW, startY + BTN_SIZE + PAD * 2 },
		                  IM_COL32(30, 30, 30, 200), BTN_ROUND * 2);

		ImGui::SetCursorScreenPos({ startX + PAD, startY + PAD });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { PAD, 0.f });
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, BTN_ROUND);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });

		auto OpBtn = [&](const char* label, int op)
		{
			bool active = (gizmoType == op);
			if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			if (ImGui::Button(label, { BTN_SIZE, BTN_SIZE }))
				gizmoType = (gizmoType == op) ? -1 : op;   // toggle off with same button
			if (active) ImGui::PopStyleColor();
			ImGui::SameLine();
		};

		OpBtn("T", ImGuizmo::OPERATION::TRANSLATE);
		OpBtn("R", ImGuizmo::OPERATION::ROTATE);
		OpBtn("S", ImGuizmo::OPERATION::SCALE);
		OpBtn("U", ImGuizmo::OPERATION::UNIVERSAL);

		// Visual separator
		ImVec2 sepCursor = ImGui::GetCursorScreenPos();
		dl->AddLine({ sepCursor.x + 1.f, startY + 4.f },
		            { sepCursor.x + 1.f, startY + BTN_SIZE + PAD * 2 - 4.f },
		            IM_COL32(80, 80, 80, 255));
		ImGui::SetCursorScreenPos({ sepCursor.x + SEPARATOR, sepCursor.y });

		auto ModeBtn = [&](const char* label, int mode)
		{
			bool active = (gizmoMode == mode);
			if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			if (ImGui::Button(label, { BTN_SIZE, BTN_SIZE }))
				gizmoMode = mode;
			if (active) ImGui::PopStyleColor();
			ImGui::SameLine();
		};

		ModeBtn("L", 0);   // LOCAL
		ModeBtn("W", 1);   // WORLD

		ImGui::PopStyleVar(3);
	}

	void ViewportPanel::OnImGuiRender()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		ImGui::PopStyleVar();

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
			if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("PREFAB_ITEM"))
			{
				if (OnPrefabDrop)
					OnPrefabDrop(std::filesystem::path(static_cast<const wchar_t*>(p->Data)));
			}
			ImGui::EndDragDropTarget();
		}

		m_Focused = ImGui::IsWindowFocused();
		// AllowWhenBlockedByActiveItem: stays true while RMB is held (active item = drag).
		// AllowWhenBlockedByPopup: stays true when a popup is open elsewhere.
		m_Hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem |
		                                   ImGuiHoveredFlags_AllowWhenBlockedByPopup);

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && m_Hovered && !m_Focused)
		{
			ImGui::SetWindowFocus();
			m_Focused = true;
		}

		// Toolbar sits on top of the viewport image
		DrawToolbar(m_GizmoType, m_GizmoMode);

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

		// Always manipulate WorldTransform — gizmo lives in world space
		glm::mat4 worldTransform = tc.WorldTransform;

		bool snap = Engine::Input::IsKeyPressed(Engine::KeyBoard::LeftControl);
		float snapValue  = (m_GizmoType == ImGuizmo::OPERATION::ROTATE) ? 5.0f : 0.5f;
		float snapValues[3] = { snapValue, snapValue, snapValue };

		ImGuizmo::MODE imguizmoMode = (m_GizmoMode == 1) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

		ImGuizmo::Manipulate(
			glm::value_ptr(m_CameraManager->GetActiveCamera()->GetView()),
			glm::value_ptr(m_CameraManager->GetActiveCamera()->GetProjection()),
			static_cast<ImGuizmo::OPERATION>(m_GizmoType), imguizmoMode,
			glm::value_ptr(worldTransform), nullptr, snap ? snapValues : nullptr);

		if (ImGuizmo::IsUsing())
		{
			// worldTransform is now the desired new world matrix.
			// Convert back to local: local = inverse(parentWorld) * newWorld
			glm::mat4 parentWorld = glm::mat4(1.0f);

			Scene* scene = selected.GetScene();
			if (scene)
			{
				Engine::EntityID id = selected;
				const Engine::SceneNode* node = scene->GetSceneGraph().GetNode(id);
				if (node && node->Parent != Engine::INVALID_ENTITY)
				{
					Entity parentEnt{ node->Parent, scene };
					if (parentEnt)
						parentWorld = parentEnt.GetComponent<TransformComponent>().WorldTransform;
				}
			}

			glm::mat4 newLocal = glm::inverse(parentWorld) * worldTransform;
			glm::vec3 translation, rotation, scale;
			Engine::Math::DecomposeTransform(newLocal, translation, rotation, scale);

			glm::vec3 deltaRotation = rotation - tc.Rotation;
			tc.Translation = translation;
			tc.Rotation   += deltaRotation;
			tc.Scale       = scale;
		}
	}
}
