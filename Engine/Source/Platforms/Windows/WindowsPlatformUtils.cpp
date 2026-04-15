#include "kepch.h"

#if defined(PLATFORM_WINDOWS)

#include "Engine/Utils/PlatformUtils.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

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

	std::optional<std::filesystem::path> PlatformUtils::OpenFileDialog(const char* filter, const std::filesystem::path& initialDir)
	{
		OPENFILENAMEW ofn{};
		wchar_t szFile[MAX_PATH] = {};

		// Convert filter (char*) → wchar_t buffer
		// filter uses null-separated pairs; count total bytes to convert properly
		int filterLen = 0;
		while (!(filter[filterLen] == '\0' && filter[filterLen + 1] == '\0'))
			++filterLen;
		filterLen += 2; // include double-null terminator

		std::wstring wFilter(filterLen, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, filter, filterLen, wFilter.data(), filterLen);

		const std::wstring wInitialDir = initialDir.empty() ? L"" : initialDir.wstring();

		ofn.lStructSize			= sizeof(ofn);
		ofn.hwndOwner			= nullptr;
		ofn.lpstrFilter			= wFilter.data();
		ofn.lpstrFile			= szFile;
		ofn.nMaxFile			= MAX_PATH;
		ofn.lpstrInitialDir		= wInitialDir.empty() ? nullptr : wInitialDir.c_str();
		ofn.Flags				= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameW(&ofn))
			return std::filesystem::path(szFile);

		return std::nullopt;
	}

	std::optional<std::filesystem::path> PlatformUtils::SaveFileDialog(const char* filter, const std::filesystem::path& defaultName, const std::filesystem::path& initialDir)
	{
		OPENFILENAMEW ofn{};
		wchar_t szFile[MAX_PATH] = {};

		if (!defaultName.empty())
		{
			const std::wstring wDefault = defaultName.wstring();
			wcsncpy_s(szFile, wDefault.c_str(), MAX_PATH - 1);
		}

		int filterLen = 0;
		while (!(filter[filterLen] == '\0' && filter[filterLen + 1] == '\0'))
			++filterLen;
		filterLen += 2;

		std::wstring wFilter(filterLen, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, filter, filterLen, wFilter.data(), filterLen);

		const std::wstring wInitialDir = initialDir.empty() ? L"" : initialDir.wstring();

		ofn.lStructSize       = sizeof(ofn);
		ofn.hwndOwner         = nullptr;
		ofn.lpstrFilter       = wFilter.data();
		ofn.lpstrFile         = szFile;
		ofn.nMaxFile          = MAX_PATH;
		ofn.lpstrInitialDir   = wInitialDir.empty() ? nullptr : wInitialDir.c_str();
		ofn.Flags             = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		if (GetSaveFileNameW(&ofn))
			return std::filesystem::path(szFile);

		return std::nullopt;
	}
}

#endif