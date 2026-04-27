#include "kepch.h"

#ifdef PLATFORM_WINDOWS

#include "Engine/Core/Application.h"

#include "Engine/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

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

	std::filesystem::path PlatformUtils::OpenFileDialog(const char* filter, const std::filesystem::path& initialDir)
	{
		OPENFILENAMEA ofn{};
		CHAR szFile[MAX_PATH] = {};
		
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window(Application::Get().GetWindow()->GetHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		
		std::string initialDirStr = initialDir.string();
		ofn.lpstrInitialDir = initialDir.empty() ? nullptr : initialDirStr.c_str();
		
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn))
			return { ofn.lpstrFile };

		return {};
	}

	std::filesystem::path PlatformUtils::SaveFileDialog(const char* filter, const std::filesystem::path& defaultName, const std::filesystem::path& initialDir)
	{
		OPENFILENAMEA ofn{};
		CHAR szFile[MAX_PATH] = {};
		
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window(Application::Get().GetWindow()->GetHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		
		std::string initialDirStr = initialDir.string();
		ofn.lpstrInitialDir = initialDir.empty() ? nullptr : initialDirStr.c_str();
		
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn))
			return { ofn.lpstrFile };

		return {};
	}

}

#endif