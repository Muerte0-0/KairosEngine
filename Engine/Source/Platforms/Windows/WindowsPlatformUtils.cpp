#include "kepch.h"

#if defined(PLATFORM_WINDOWS)

#include "Engine/Utils/PlatformUtils.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace Engine
{
	std::optional<std::filesystem::path> PlatformUtils::GetExecutablePath()
	{
		std::wstring buffer(MAX_PATH, L'\0');

		while (true)
		{
			const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

			if (length == 0)
				return std::nullopt;

			if (length < buffer.size())
			{
				buffer.resize(length);
				return std::filesystem::path(buffer);
			}

			buffer.resize(buffer.size() * 2);
		}
	}

	std::optional<std::filesystem::path> PlatformUtils::FindWorkspaceRoot(std::filesystem::path startPath)
	{
		if (startPath.empty())
			return std::nullopt;

		if (std::filesystem::is_regular_file(startPath))
			startPath = startPath.parent_path();

		for (std::filesystem::path current = std::filesystem::weakly_canonical(startPath);
			!current.empty();
			current = current.parent_path())
		{
			if (std::filesystem::exists(current / "KairosEngine-Setup.lua"))
				return current;

			if (current == current.root_path())
				break;
		}

		return std::nullopt;
	}

	std::optional<std::filesystem::path> PlatformUtils::ResolveWorkspaceRoot()
	{
		if (const auto executablePath = GetExecutablePath())
		{
			if (const auto root = FindWorkspaceRoot(*executablePath))
				return root;
		}

		return FindWorkspaceRoot(std::filesystem::current_path());
	}
}

#endif