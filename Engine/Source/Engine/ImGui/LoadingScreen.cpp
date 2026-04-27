#include "kepch.h"
#include "LoadingScreen.h"

#include "imgui.h"
#include <glm/glm.hpp>
#include <cmath>

namespace Engine
{
	// -----------------------------------------------------------------------
	// UIAnimationSystem
	// -----------------------------------------------------------------------
	void UIAnimationSystem::Update(float dt)
	{
		m_DeltaTime = dt;
	}

	float UIAnimationSystem::Get(const std::string& id) const
	{
		auto it = m_Values.find(id);
		return (it != m_Values.end()) ? it->second : 0.0f;
	}

	float UIAnimationSystem::Lerp(const std::string& id, float target, float speed)
	{
		float& current = m_Values[id];
		current += (target - current) * glm::clamp(speed * m_DeltaTime, 0.0f, 1.0f);
		return current;
	}

	void UIAnimationSystem::Set(const std::string& id, float value)
	{
		m_Values[id] = value;
	}

	// -----------------------------------------------------------------------
	// LoadingScreen — lifecycle
	// -----------------------------------------------------------------------
	void LoadingScreen::OnLoadingStarted()
	{
		m_LoadingStarted  = true;
		m_LoadingFinished = false;
		m_FadeOutTimer    = 0.0f;

		// Snap overlay alpha to 0 so fade-in plays from scratch.
		m_Anim.Set("OverlayAlpha", 0.0f);
		m_Anim.Set("PanelScale",   0.92f);
		m_Anim.Set("Progress",     0.0f);
	}

	void LoadingScreen::OnLoadingFinished()
	{
		if (m_LoadingFinished)
			return; // Guard: only start fade-out once — re-entry resets the timer.
		m_LoadingFinished = true;
		m_FadeOutTimer    = 0.0f;
	}

	bool LoadingScreen::IsFadingOut() const
	{
		return m_LoadingFinished && (m_FadeOutTimer < FADE_OUT_DURATION);
	}

	void LoadingScreen::Update(float dt)
	{
		m_Anim.Update(dt);
		if (m_LoadingFinished)
			m_FadeOutTimer += dt;
	}

	// -----------------------------------------------------------------------
	// LoadingScreen — rendering entry point
	// -----------------------------------------------------------------------
	void LoadingScreen::Render(LoadingPhase phase, float progress, const std::string& statusText)
	{
		if (!m_LoadingStarted)
			return;

		ImGuiIO& io = ImGui::GetIO();

		// Block all input while the loading screen is visible.
		io.WantCaptureMouse    = true;
		io.WantCaptureKeyboard = true;

		const float fadeTarget    = m_LoadingFinished
			? glm::max(0.0f, 1.0f - m_FadeOutTimer / FADE_OUT_DURATION)
			: 1.0f;

		const float animSpeed     = (phase == LoadingPhase::SceneTransition) ? 14.0f : 8.0f;
		const float overlayAlpha  = m_Anim.Lerp("OverlayAlpha", fadeTarget, animSpeed);
		const float panelScale    = m_Anim.Lerp("PanelScale",   fadeTarget > 0.0f ? 1.0f : 0.92f, animSpeed);
		const float panelAlpha    = overlayAlpha * panelScale; // rough compound

		if (overlayAlpha < 0.01f && m_LoadingFinished)
			return;

		RenderOverlay(overlayAlpha);
		RenderPanel(phase, progress, statusText, panelAlpha);
	}

	// -----------------------------------------------------------------------
	// LoadingScreen — fullscreen overlay
	// -----------------------------------------------------------------------
	void LoadingScreen::RenderOverlay(float overlayAlpha)
	{
		const ImGuiViewport* vp = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(vp->Pos);
		ImGui::SetNextWindowSize(vp->Size);
		ImGui::SetNextWindowBgAlpha(0.82f * overlayAlpha);

		constexpr ImGuiWindowFlags kOverlayFlags =
			ImGuiWindowFlags_NoDecoration     |
			ImGuiWindowFlags_NoMove           |
			ImGuiWindowFlags_NoSavedSettings  |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNav            |
			ImGuiWindowFlags_NoInputs;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin("##LoadingOverlay", nullptr, kOverlayFlags);
		ImGui::PopStyleVar(2);
		ImGui::End();
	}

	// -----------------------------------------------------------------------
	// LoadingScreen — centered popup panel
	// -----------------------------------------------------------------------
	void LoadingScreen::RenderPanel(LoadingPhase phase, float progress,
	                                const std::string& statusText, float panelAlpha)
	{
		const ImGuiViewport* vp  = ImGui::GetMainViewport();
		const ImVec2 displaySize = vp->Size;

		constexpr float kPanelW = 460.0f;
		constexpr float kPanelH = 180.0f;

		const ImVec2 panelPos = {
			vp->Pos.x + (displaySize.x - kPanelW) * 0.5f,
			vp->Pos.y + (displaySize.y - kPanelH) * 0.5f
		};

		ImGui::SetNextWindowPos(panelPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize({ kPanelW, kPanelH }, ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(panelAlpha);

		constexpr ImGuiWindowFlags kPanelFlags =
			ImGuiWindowFlags_NoDecoration    |
			ImGuiWindowFlags_NoMove          |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoNav           |
			ImGuiWindowFlags_NoInputs;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		ImGui::Begin("##LoadingPanel", nullptr, kPanelFlags);
		ImGui::PopStyleVar(2);

		const float time = static_cast<float>(ImGui::GetTime());

		RenderStatusText(phase, statusText, time, panelAlpha);
		ImGui::Spacing();
		RenderProgressBar(m_Anim.Lerp("Progress", progress, 5.0f), panelAlpha);

		ImGui::End();
	}

	// -----------------------------------------------------------------------
	// LoadingScreen — progress bar
	// -----------------------------------------------------------------------
	void LoadingScreen::RenderProgressBar(float smoothProgress, float panelAlpha)
	{
		constexpr float kBarH = 8.0f;

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
			ImVec4(0.38f, 0.70f, 1.0f, panelAlpha));
		ImGui::PushStyleColor(ImGuiCol_FrameBg,
			ImVec4(0.15f, 0.15f, 0.18f, panelAlpha * 0.8f));

		const float barWidth = ImGui::GetContentRegionAvail().x;
		ImGui::ProgressBar(smoothProgress, { barWidth, kBarH }, "");

		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();

		// Percentage text
		ImGui::SameLine(0.0f, 8.0f);
		ImGui::TextDisabled("%d%%", static_cast<int>(smoothProgress * 100.0f));
	}

	// -----------------------------------------------------------------------
	// LoadingScreen — status / title text
	// -----------------------------------------------------------------------
	void LoadingScreen::RenderStatusText(LoadingPhase phase, const std::string& statusText,
	                                     float time, float panelAlpha)
	{
		// Title
		const char* title = (phase == LoadingPhase::EditorStartup)
			? "Initializing Engine"
			: "Loading Scene";

		// Pulsing alpha for title
		const float pulse = 0.85f + 0.15f * std::sin(time * 2.5f);
		ImGui::PushStyleColor(ImGuiCol_Text,
			ImVec4(1.0f, 1.0f, 1.0f, panelAlpha * pulse));
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(title).x) * 0.5f);
		ImGui::TextUnformatted(title);
		ImGui::PopStyleColor();

		// Animated dots
		const int dots      = (static_cast<int>(time * 2.0f) % 4);
		const char* dotStr  = (dots == 0) ? ""   :
		                      (dots == 1) ? "."  :
		                      (dots == 2) ? ".." : "...";

		ImGui::Spacing();

		// Status sub-text (startup phase shows current step)
		if (phase == LoadingPhase::EditorStartup && !statusText.empty())
		{
			const std::string sub = statusText + dotStr;
			ImGui::PushStyleColor(ImGuiCol_Text,
				ImVec4(0.7f, 0.7f, 0.7f, panelAlpha * 0.9f));
			ImGui::SetCursorPosX(
				(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(sub.c_str()).x) * 0.5f);
			ImGui::TextUnformatted(sub.c_str());
			ImGui::PopStyleColor();
		}
		else
		{
			const std::string sub = std::string("Loading") + dotStr;
			ImGui::PushStyleColor(ImGuiCol_Text,
				ImVec4(0.6f, 0.6f, 0.6f, panelAlpha * 0.8f));
			ImGui::SetCursorPosX(
				(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(sub.c_str()).x) * 0.5f);
			ImGui::TextUnformatted(sub.c_str());
			ImGui::PopStyleColor();
		}

		ImGui::Spacing();
	}
} // namespace Engine
