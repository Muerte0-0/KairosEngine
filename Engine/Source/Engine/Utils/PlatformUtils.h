#pragma once

#include <filesystem>
#include <optional>

namespace Engine
{
	class PlatformUtils
	{
	public:
		/**
		 * @brief Returns the full path to the running executable.
		 * @return The executable path, or std::nullopt on failure.
		 */
		[[nodiscard]] static std::optional<std::filesystem::path> GetExecutablePath();

		/**
		 * @brief Walks up from startPath until it finds a directory containing
		 *        KairosEngine-Setup.lua, which marks the workspace root.
		 * @param startPath A file or directory to begin the search from.
		 * @return The workspace root path, or std::nullopt if not found.
		 */
		[[nodiscard]] static std::optional<std::filesystem::path> FindWorkspaceRoot(std::filesystem::path startPath);

		/**
		 * @brief Resolves the workspace root by checking the executable location
		 *        first, then the current working directory.
		 * @return The workspace root path, or std::nullopt if not found.
		 */
		[[nodiscard]] static std::optional<std::filesystem::path> ResolveWorkspaceRoot();

		/**
		 * @brief Opens a native "Open File" dialog.
		 * @param filter Null-separated filter pairs: "Display\0*.ext\0All Files\0*.*\0\0"
		 * @param initialDir Starting directory. Empty = last used.
		 * @return Selected path, or std::nullopt if cancelled.
		 */
		[[nodiscard]] static std::optional<std::filesystem::path> OpenFileDialog(
			const char* filter = "All Files\0*.*\0\0",
			const std::filesystem::path& initialDir = {});

		/**
		 * @brief Opens a native "Save File" dialog.
		 * @param filter Null-separated filter pairs.
		 * @param defaultName Suggested filename.
		 * @param initialDir Starting directory. Empty = last used.
		 * @return Selected path, or std::nullopt if cancelled.
		 */
		[[nodiscard]] static std::optional<std::filesystem::path> SaveFileDialog(
			const char* filter = "All Files\0*.*\0\0",
			const std::filesystem::path& defaultName = {},
			const std::filesystem::path& initialDir = {});
	};
}