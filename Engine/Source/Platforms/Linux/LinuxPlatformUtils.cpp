#include "kepch.h"

#if defined(PLATFORM_LINUX)

#include "Engine/Utils/PlatformUtils.h"

#include <unistd.h>
#include <linux/limits.h>

namespace Engine
{
	std::optional<std::filesystem::path> PlatformUtils::GetExecutablePath()
	{
		std::vector<char> buffer(PATH_MAX, '\0');

		while (true)
		{
			const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size());

			if (length < 0)
				return std::nullopt;

			if (static_cast<size_t>(length) < buffer.size())
				return std::filesystem::path(std::string(buffer.data(), static_cast<size_t>(length)));

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