#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ShaderCompiler
{
	struct Options
	{
		std::filesystem::path SourcePath;
		std::filesystem::path OutputPath;
		std::filesystem::path CachePath;
		std::filesystem::path SlangCompilerPath;
		std::vector<std::filesystem::path> IncludeDirectories;
		std::vector<std::string> EntryPoints;
		std::string Profile = "spirv_1_4";
		bool ForceRebuild = false;
		bool CheckOnly = false;
		bool ShowHelp = false;
		bool Verbose = false;
	};

	int Run(int argc, char** argv);
}
