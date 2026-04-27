#pragma once

namespace Engine
{
	enum class EngineState
	{
		None = 0,
		Loading,
		Running
	};

	enum class LoadingPhase
	{
		EditorStartup,
		SceneTransition
	};
}
