#pragma once

namespace Engine
{
	// Single source of truth for engine runtime state.
	// Loading is NOT a mode — use LoadingSystem::IsLoading() for that.
	enum class EngineMode : uint8_t
	{
		Editor = 0,
		Play,
		Paused
	};
}
