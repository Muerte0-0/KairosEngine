#pragma once

#include <string>
#include <unordered_map>

#include "Engine/Core/EngineState.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// UIAnimationSystem
	// Lightweight lerp-based animation state store (ImAnim-compatible API).
	// All values stored per string ID. Call Update() once per frame.
	// -----------------------------------------------------------------------
	class UIAnimationSystem
	{
	public:
		void Update(float dt);

		// Returns current animated value for 'id' (initializes to 0 on first access).
		float Get(const std::string& id) const;

		// Lerps current value toward 'target' and returns the new value.
		float Lerp(const std::string& id, float target, float speed = 10.0f);

		// Instantly set a value (e.g. to reset an animation).
		void Set(const std::string& id, float value);

	private:
		mutable std::unordered_map<std::string, float> m_Values;
		float m_DeltaTime = 0.0f;
	};

	// -----------------------------------------------------------------------
	// LoadingScreen
	// Renders a full-screen ImGui modal that blocks all input.
	// Call Render() inside your ImGui frame each tick while loading.
	// -----------------------------------------------------------------------
	class LoadingScreen
	{
	public:
		LoadingScreen() = default;

		// Call once per frame while EngineState == Loading.
		// 'phase'    : controls text flavour and animation speed.
		// 'progress' : [0, 1] raw progress from LoadingSystem.
		// 'statusText': current step description (startup phase only).
		void Render(LoadingPhase phase, float progress, const std::string& statusText);

		// Notify the loading screen that loading has started / finished so
		// it can drive fade-in / fade-out animations.
		void OnLoadingStarted();
		void OnLoadingFinished();

		// True while the fade-out animation is still playing after loading completes.
		bool IsFadingOut() const;

		void Update(float dt);

	private:
		UIAnimationSystem m_Anim;

		bool  m_LoadingStarted  = false;
		bool  m_LoadingFinished = false;
		float m_FadeOutTimer    = 0.0f;

		static constexpr float FADE_OUT_DURATION = 0.35f;

		void RenderOverlay(float overlayAlpha);
		void RenderPanel(LoadingPhase phase, float progress,
		                 const std::string& statusText, float panelAlpha);
		void RenderProgressBar(float smoothProgress, float panelAlpha);
		void RenderStatusText(LoadingPhase phase, const std::string& statusText,
		                      float time, float panelAlpha);

		// Overlay Mode specific
		void RenderOverlayPopup(LoadingPhase phase, float progress,
		                        const std::string& statusText, float panelAlpha);
	};
}
