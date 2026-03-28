#include "ShaderCompiler.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace ShaderCompiler
{
	namespace
	{
		struct StageInfo
		{
			std::string SlangStage;
			std::string OutputSuffix;
		};

		struct EntryPointInfo
		{
			std::string Name;
			StageInfo Stage;
		};

		struct ShaderRecord
		{
			std::string RecordKey;
			std::string SourcePath;
			std::string OutputPath;
			std::string SourceHash;
			std::string SourceWriteTimeUtc;
			std::string CompiledAtUtc;
			std::string Stage;
			std::string EntryPoint;
			std::string Profile;
		};

		struct Manifest
		{
			std::string SlangCompilerPath;
			std::string Target = "spirv";
			std::unordered_map<std::string, ShaderRecord> Records;
		};

		struct ProcessResult
		{
			int ExitCode = -1;
			std::string Output;
		};

		const std::unordered_map<std::string, StageInfo> kStageSuffixMap = {
			{ ".vert", { "vertex", ".vert.spv" } },
			{ ".vs", { "vertex", ".vert.spv" } },
			{ ".frag", { "fragment", ".frag.spv" } },
			{ ".ps", { "fragment", ".frag.spv" } },
			{ ".comp", { "compute", ".comp.spv" } },
			{ ".cs", { "compute", ".comp.spv" } },
			{ ".geom", { "geometry", ".geom.spv" } },
			{ ".gs", { "geometry", ".geom.spv" } },
			{ ".tesc", { "hull", ".tesc.spv" } },
			{ ".hs", { "hull", ".tesc.spv" } },
			{ ".tese", { "domain", ".tese.spv" } },
			{ ".ds", { "domain", ".tese.spv" } },
			{ ".mesh", { "mesh", ".mesh.spv" } },
			{ ".task", { "amplification", ".task.spv" } },
			{ ".rgen", { "raygeneration", ".rgen.spv" } },
			{ ".rmiss", { "miss", ".rmiss.spv" } },
			{ ".rchit", { "closesthit", ".rchit.spv" } },
			{ ".rahit", { "anyhit", ".rahit.spv" } },
			{ ".rint", { "intersection", ".rint.spv" } },
			{ ".callable", { "callable", ".callable.spv" } }
		};

		const std::unordered_map<std::string, StageInfo> kStageNameMap = {
			{ "vertex", { "vertex", ".vert.spv" } },
			{ "fragment", { "fragment", ".frag.spv" } },
			{ "compute", { "compute", ".comp.spv" } },
			{ "geometry", { "geometry", ".geom.spv" } },
			{ "hull", { "hull", ".tesc.spv" } },
			{ "domain", { "domain", ".tese.spv" } },
			{ "mesh", { "mesh", ".mesh.spv" } },
			{ "amplification", { "amplification", ".task.spv" } },
			{ "raygeneration", { "raygeneration", ".rgen.spv" } },
			{ "miss", { "miss", ".rmiss.spv" } },
			{ "closesthit", { "closesthit", ".rchit.spv" } },
			{ "anyhit", { "anyhit", ".rahit.spv" } },
			{ "intersection", { "intersection", ".rint.spv" } },
			{ "callable", { "callable", ".callable.spv" } }
		};

		std::string ToLowerCopy(std::string value)
		{
			std::ranges::transform(value, value.begin(), [](const unsigned char character)
			{
				return static_cast<char>(std::tolower(character));
			});
			return value;
		}

		std::string NormalizePathString(const fs::path& path)
		{
			return fs::weakly_canonical(path).generic_string();
		}

		std::string QuoteArgument(const std::string& value)
		{
			std::string quoted = "\"";
			for (const char character : value)
			{
				if (character == '"')
				{
					quoted += "\\\"";
				}
				else
				{
					quoted += character;
				}
			}
			quoted += "\"";
			return quoted;
		}

		std::string JoinCommand(const std::vector<std::string>& arguments)
		{
			std::ostringstream builder;
			for (size_t index = 0; index < arguments.size(); ++index)
			{
				if (index > 0)
				{
					builder << ' ';
				}
				builder << QuoteArgument(arguments[index]);
			}
			return builder.str();
		}

		ProcessResult RunProcess(const std::vector<std::string>& arguments)
		{
			ProcessResult result;

#ifdef _WIN32
			const std::string innerCommand = JoinCommand(arguments) + " 2>&1";
			const std::string command = "cmd /d /s /c \"" + innerCommand + "\"";
			FILE* pipe = _popen(command.c_str(), "r");
#else
			const std::string command = JoinCommand(arguments) + " 2>&1";
			FILE* pipe = popen(command.c_str(), "r");
#endif
			if (!pipe)
			{
				result.Output = "Failed to start process.";
				return result;
			}

			std::array<char, 512> buffer {};
			while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
			{
				result.Output += buffer.data();
			}

#ifdef _WIN32
			result.ExitCode = _pclose(pipe);
#else
			result.ExitCode = pclose(pipe);
#endif
			return result;
		}

		std::optional<std::string> ReadEnvironmentVariable(const char* name)
		{
#ifdef _WIN32
			char* value = nullptr;
			size_t length = 0;
			if (_dupenv_s(&value, &length, name) != 0 || value == nullptr)
			{
				return std::nullopt;
			}

			std::string result(value);
			free(value);
			return result;
#else
			if (const char* value = std::getenv(name))
			{
				return std::string(value);
			}
			return std::nullopt;
#endif
		}

		fs::path ResolveDefaultSlangCompilerPath()
		{
			const auto vulkanSdk = ReadEnvironmentVariable("VULKAN_SDK");
			if (!vulkanSdk.has_value())
			{
				return {};
			}

#ifdef _WIN32
			return fs::path(*vulkanSdk) / "Bin" / "slangc.exe";
#else
			return fs::path(*vulkanSdk) / "bin" / "slangc";
#endif
		}

		std::string ReadFileText(const fs::path& filePath)
		{
			std::ifstream stream(filePath, std::ios::binary);
			if (!stream.is_open())
			{
				throw std::runtime_error("Failed to open file: " + filePath.string());
			}

			std::ostringstream builder;
			builder << stream.rdbuf();
			return builder.str();
		}

		std::string HashFileContents(const fs::path& filePath)
		{
			const std::string contents = ReadFileText(filePath);
			constexpr uint64_t offsetBasis = 14695981039346656037ull;
			constexpr uint64_t prime = 1099511628211ull;

			uint64_t hash = offsetBasis;
			for (const unsigned char byte : contents)
			{
				hash ^= byte;
				hash *= prime;
			}

			std::ostringstream builder;
			builder << std::hex << std::setfill('0') << std::setw(16) << hash;
			return builder.str();
		}

		std::string FileTimeToUtcString(const fs::file_time_type time)
		{
			const auto systemNow = std::chrono::system_clock::now();
			const auto fileNow = fs::file_time_type::clock::now();
			const auto translated = systemNow + std::chrono::duration_cast<std::chrono::system_clock::duration>(time - fileNow);
			const std::time_t timeValue = std::chrono::system_clock::to_time_t(translated);

			std::tm utcTime {};
#ifdef _WIN32
			gmtime_s(&utcTime, &timeValue);
#else
			gmtime_r(&timeValue, &utcTime);
#endif

			std::ostringstream builder;
			builder << std::put_time(&utcTime, "%Y-%m-%dT%H:%M:%SZ");
			return builder.str();
		}

		std::string GetCurrentUtcString()
		{
			const std::time_t timeValue = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::tm utcTime {};
#ifdef _WIN32
			gmtime_s(&utcTime, &timeValue);
#else
			gmtime_r(&timeValue, &utcTime);
#endif

			std::ostringstream builder;
			builder << std::put_time(&utcTime, "%Y-%m-%dT%H:%M:%SZ");
			return builder.str();
		}

		void PrintUsage()
		{
			std::cout
				<< "ShaderCompiler\n"
				<< "  --source <path>          Source .slang file or directory\n"
				<< "  --output <path>          Output file or directory for generated .spv files\n"
				<< "  --cache <path>           Manifest file path (default: <output>/.shadercompiler-cache.json)\n"
				<< "  --slangc <path>          Explicit path to slangc executable\n"
				<< "  --include <path>         Additional include directory, can be repeated\n"
				<< "  --entry <name>           Entry point name, can be repeated\n"
				<< "  --profile <profile>      Slang profile (default: spirv_1_4)\n"
				<< "  --force                  Recompile all shaders\n"
				<< "  --check                  Do not compile, only report stale/missing outputs\n"
				<< "  --verbose                Print compiler commands and skips\n";
		}

		bool TryParseOptions(int argc, char** argv, Options& options)
		{
			for (int index = 1; index < argc; ++index)
			{
				const std::string_view argument = argv[index];

				auto readValue = [&](const std::string_view name) -> std::optional<std::string>
				{
					if (index + 1 >= argc)
					{
						std::cerr << "Missing value for " << name << '\n';
						return std::nullopt;
					}
					return std::string(argv[++index]);
				};

				if (argument == "--source")
				{
					const auto value = readValue(argument);
					if (!value.has_value())
					{
						return false;
					}
					options.SourcePath = *value;
				}
				else if (argument == "--output")
				{
					const auto value = readValue(argument);
					if (!value.has_value())
					{
						return false;
					}
					options.OutputPath = *value;
				}
				else if (argument == "--cache")
				{
					const auto value = readValue(argument);
					if (!value.has_value())
					{
						return false;
					}
					options.CachePath = *value;
				}
				else if (argument == "--slangc")
				{
					const auto value = readValue(argument);
					if (!value.has_value())
					{
						return false;
					}
					options.SlangCompilerPath = *value;
				}
				else if (argument == "--include")
				{
					const auto value = readValue(argument);
					if (!value.has_value())
					{
						return false;
					}
					options.IncludeDirectories.emplace_back(*value);
				}
				else if (argument == "--entry")
				{
					const auto value = readValue(argument);
					if (!value.has_value())
					{
						return false;
					}
					options.EntryPoints.emplace_back(*value);
				}
				else if (argument == "--profile")
				{
					const auto value = readValue(argument);
					if (!value.has_value())
					{
						return false;
					}
					options.Profile = *value;
				}
				else if (argument == "--force")
				{
					options.ForceRebuild = true;
				}
				else if (argument == "--check")
				{
					options.CheckOnly = true;
				}
				else if (argument == "--verbose")
				{
					options.Verbose = true;
				}
				else if (argument == "--help" || argument == "-h")
				{
					options.ShowHelp = true;
					PrintUsage();
					return false;
				}
				else
				{
					std::cerr << "Unknown argument: " << argument << '\n';
					return false;
				}
			}

			if (options.SourcePath.empty())
			{
				std::cerr << "Missing required argument: --source <path>\n";
				return false;
			}

			if (options.SlangCompilerPath.empty())
			{
				options.SlangCompilerPath = ResolveDefaultSlangCompilerPath();
			}

			return true;
		}

		std::optional<StageInfo> InferStageInfoFromFileName(const fs::path& sourceFile)
		{
			const std::string stem = ToLowerCopy(sourceFile.stem().string());
			for (const auto& [suffix, info] : kStageSuffixMap)
			{
				if (stem.ends_with(suffix))
				{
					return info;
				}
			}
			return std::nullopt;
		}

		std::vector<EntryPointInfo> DiscoverAnnotatedEntryPoints(const fs::path& sourceFile)
		{
			std::vector<EntryPointInfo> entryPoints;
			const std::string source = ReadFileText(sourceFile);
			const std::regex pattern(R"__REGEX(\[shader\("([A-Za-z]+)"\)\]\s*[\w:<>,\s\[\]]+\s+([A-Za-z_]\w*)\s*\()__REGEX", std::regex::ECMAScript);

			for (std::sregex_iterator iterator(source.begin(), source.end(), pattern), end; iterator != end; ++iterator)
			{
				const std::string stageName = ToLowerCopy((*iterator)[1].str());
				const std::string entryName = (*iterator)[2].str();

				const auto stageIterator = kStageNameMap.find(stageName);
				if (stageIterator != kStageNameMap.end())
				{
					entryPoints.push_back({ entryName, stageIterator->second });
				}
			}

			return entryPoints;
		}

		std::vector<EntryPointInfo> ResolveEntryPoints(const Options& options, const fs::path& sourceFile)
		{
			const std::vector<EntryPointInfo> discoveredEntries = DiscoverAnnotatedEntryPoints(sourceFile);
			if (!options.EntryPoints.empty())
			{
				std::vector<EntryPointInfo> requestedEntries;
				for (const std::string& requestedEntry : options.EntryPoints)
				{
					const auto discoveredIterator = std::ranges::find_if(discoveredEntries, [&](const EntryPointInfo& entry)
					{
						return entry.Name == requestedEntry;
					});

					if (discoveredIterator != discoveredEntries.end())
					{
						requestedEntries.push_back(*discoveredIterator);
						continue;
					}

					const auto stageInfo = InferStageInfoFromFileName(sourceFile);
					if (!stageInfo.has_value())
					{
						throw std::runtime_error(
							"Entry point '" + requestedEntry + "' was requested for " + sourceFile.string() +
							", but the stage could not be resolved. Add [shader(\"...\")] attributes or rename the file with a stage suffix.");
					}

					requestedEntries.push_back({ requestedEntry, *stageInfo });
				}

				return requestedEntries;
			}

			if (!discoveredEntries.empty())
			{
				return discoveredEntries;
			}

			const auto stageInfo = InferStageInfoFromFileName(sourceFile);
			if (stageInfo.has_value())
			{
				return { EntryPointInfo { "main", *stageInfo } };
			}

			throw std::runtime_error(
				"Unable to resolve entry points for " + sourceFile.string() +
				". Add [shader(\"...\")] attributes or use a file name like *.vert.slang.");
		}

		std::vector<fs::path> CollectShaderFiles(const fs::path& sourcePath)
		{
			std::vector<fs::path> shaderFiles;

			if (fs::is_regular_file(sourcePath))
			{
				if (ToLowerCopy(sourcePath.extension().string()) == ".slang")
				{
					shaderFiles.push_back(fs::weakly_canonical(sourcePath));
				}
				return shaderFiles;
			}

			for (const auto& entry : fs::recursive_directory_iterator(sourcePath))
			{
				if (!entry.is_regular_file())
				{
					continue;
				}

				if (ToLowerCopy(entry.path().extension().string()) == ".slang")
				{
					shaderFiles.push_back(fs::weakly_canonical(entry.path()));
				}
			}

			std::ranges::sort(shaderFiles);
			return shaderFiles;
		}

		fs::path ResolveOutputRoot(const Options& options)
		{
			if (!options.OutputPath.empty())
			{
				return options.OutputPath;
			}

			if (fs::is_directory(options.SourcePath))
			{
				return options.SourcePath / "Compiled";
			}

			return options.SourcePath.parent_path() / "Compiled";
		}

		fs::path ResolveOutputPath(const Options& options, const fs::path& outputRoot, const fs::path& sourceFile, const EntryPointInfo& entryPoint)
		{
			if (fs::is_regular_file(options.SourcePath) && !options.OutputPath.empty() && options.OutputPath.has_extension())
			{
				return options.OutputPath;
			}

			fs::path relativePath;
			if (fs::is_directory(options.SourcePath))
			{
				relativePath = fs::relative(sourceFile, options.SourcePath);
			}
			else
			{
				relativePath = sourceFile.filename();
			}

			const std::string outputFileName = sourceFile.stem().string() + "." + entryPoint.Name + entryPoint.Stage.OutputSuffix;
			relativePath.replace_filename(outputFileName);
			return outputRoot / relativePath;
		}

		ShaderRecord ParseShaderRecord(const json& input)
		{
			ShaderRecord record;
			record.RecordKey = input.value("recordKey", "");
			record.SourcePath = input.value("sourcePath", "");
			record.OutputPath = input.value("outputPath", "");
			record.SourceHash = input.value("sourceHash", "");
			record.SourceWriteTimeUtc = input.value("sourceWriteTimeUtc", "");
			record.CompiledAtUtc = input.value("compiledAtUtc", "");
			record.Stage = input.value("stage", "");
			record.EntryPoint = input.value("entryPoint", "");
			record.Profile = input.value("profile", "");
			return record;
		}

		json SerializeShaderRecord(const ShaderRecord& record)
		{
			return json {
				{ "recordKey", record.RecordKey },
				{ "sourcePath", record.SourcePath },
				{ "outputPath", record.OutputPath },
				{ "sourceHash", record.SourceHash },
				{ "sourceWriteTimeUtc", record.SourceWriteTimeUtc },
				{ "compiledAtUtc", record.CompiledAtUtc },
				{ "stage", record.Stage },
				{ "entryPoint", record.EntryPoint },
				{ "profile", record.Profile }
			};
		}

		Manifest LoadManifest(const fs::path& manifestPath)
		{
			Manifest manifest;
			if (!fs::exists(manifestPath))
			{
				return manifest;
			}

			const auto parsed = json::parse(ReadFileText(manifestPath), nullptr, false);
			if (parsed.is_discarded())
			{
				std::cerr << "Manifest is invalid JSON, recreating: " << manifestPath << '\n';
				return manifest;
			}

			manifest.SlangCompilerPath = parsed.value("slangCompilerPath", "");
			manifest.Target = parsed.value("target", "spirv");

			const json recordsJson = parsed.value("records", json::object());
			for (auto iterator = recordsJson.begin(); iterator != recordsJson.end(); ++iterator)
			{
				manifest.Records.emplace(iterator.key(), ParseShaderRecord(iterator.value()));
			}

			return manifest;
		}

		void SaveManifest(const fs::path& manifestPath, const Manifest& manifest)
		{
			json recordsJson = json::object();
			std::map<std::string, ShaderRecord> sortedRecords(manifest.Records.begin(), manifest.Records.end());
			for (const auto& [key, record] : sortedRecords)
			{
				recordsJson[key] = SerializeShaderRecord(record);
			}

			const json output = {
				{ "version", 1 },
				{ "target", manifest.Target },
				{ "slangCompilerPath", manifest.SlangCompilerPath },
				{ "records", recordsJson }
			};

			if (manifestPath.has_parent_path())
			{
				fs::create_directories(manifestPath.parent_path());
			}

			std::ofstream stream(manifestPath, std::ios::binary | std::ios::trunc);
			stream << output.dump(4);
		}

		bool NeedsRecompile(
			const Options& options,
			const Manifest& manifest,
			const std::string& recordKey,
			const fs::path& outputFile,
			const EntryPointInfo& entryPoint,
			const std::string& sourceHash,
			const std::string& sourceWriteTimeUtc)
		{
			if (options.ForceRebuild)
			{
				return true;
			}

			if (!fs::exists(outputFile))
			{
				return true;
			}

			const auto iterator = manifest.Records.find(recordKey);
			if (iterator == manifest.Records.end())
			{
				return true;
			}

			const ShaderRecord& record = iterator->second;
			if (record.OutputPath != NormalizePathString(outputFile))
			{
				return true;
			}

			if (record.SourceHash != sourceHash || record.SourceWriteTimeUtc != sourceWriteTimeUtc)
			{
				return true;
			}

			if (record.EntryPoint != entryPoint.Name || record.Stage != entryPoint.Stage.SlangStage || record.Profile != options.Profile)
			{
				return true;
			}

			if (manifest.SlangCompilerPath != NormalizePathString(options.SlangCompilerPath))
			{
				return true;
			}

			return false;
		}

		int CompileEntryPoint(
			const Options& options,
			Manifest& manifest,
			const std::string& recordKey,
			const fs::path& sourceFile,
			const fs::path& outputFile,
			const EntryPointInfo& entryPoint,
			const std::string& sourceHash,
			const std::string& sourceWriteTimeUtc)
		{
			std::vector<std::string> arguments;
			arguments.emplace_back(options.SlangCompilerPath.string());
			arguments.emplace_back(sourceFile.string());
			arguments.emplace_back("-target");
			arguments.emplace_back("spirv");
			arguments.emplace_back("-profile");
			arguments.emplace_back(options.Profile);
			arguments.emplace_back("-emit-spirv-directly");
			arguments.emplace_back("-fvk-use-entrypoint-name");
			arguments.emplace_back("-entry");
			arguments.emplace_back(entryPoint.Name);
			arguments.emplace_back("-stage");
			arguments.emplace_back(entryPoint.Stage.SlangStage);
			arguments.emplace_back("-o");
			arguments.emplace_back(outputFile.string());
			arguments.emplace_back("-depfile");
			arguments.emplace_back(outputFile.string() + ".d");

			for (const fs::path& includeDirectory : options.IncludeDirectories)
			{
				arguments.emplace_back("-I");
				arguments.emplace_back(includeDirectory.string());
			}

			if (options.Verbose)
			{
				std::cout << "[compile] " << sourceFile << " (" << entryPoint.Name << ")\n";
				std::cout << JoinCommand(arguments) << '\n';
			}
			else
			{
				std::cout << "[compile] " << sourceFile << " (" << entryPoint.Name << ") -> " << outputFile << '\n';
			}

			fs::create_directories(outputFile.parent_path());
			const ProcessResult result = RunProcess(arguments);
			if (!result.Output.empty())
			{
				std::cout << result.Output;
				if (!result.Output.ends_with('\n'))
				{
					std::cout << '\n';
				}
			}

			if (result.ExitCode != 0)
			{
				std::cerr << "Compilation failed for " << sourceFile << " (" << entryPoint.Name << ")\n";
				return 1;
			}

			ShaderRecord record;
			record.RecordKey = recordKey;
			record.SourcePath = NormalizePathString(sourceFile);
			record.OutputPath = NormalizePathString(outputFile);
			record.SourceHash = sourceHash;
			record.SourceWriteTimeUtc = sourceWriteTimeUtc;
			record.CompiledAtUtc = GetCurrentUtcString();
			record.Stage = entryPoint.Stage.SlangStage;
			record.EntryPoint = entryPoint.Name;
			record.Profile = options.Profile;
			manifest.Records[recordKey] = std::move(record);
			manifest.SlangCompilerPath = NormalizePathString(options.SlangCompilerPath);

			return 0;
		}
	}

	int Run(int argc, char** argv)
	{
		Options options;
		if (!TryParseOptions(argc, argv, options))
		{
			return options.ShowHelp ? 0 : 1;
		}

		try
		{
			options.SourcePath = fs::weakly_canonical(options.SourcePath);

			if (!fs::exists(options.SourcePath))
			{
				std::cerr << "Source path does not exist: " << options.SourcePath << '\n';
				return 1;
			}

			if (options.SlangCompilerPath.empty())
			{
				std::cerr << "Unable to resolve slangc. Pass --slangc <path> or set VULKAN_SDK.\n";
				return 1;
			}

			options.SlangCompilerPath = fs::weakly_canonical(options.SlangCompilerPath);
			if (!fs::exists(options.SlangCompilerPath))
			{
				std::cerr << "slangc executable does not exist: " << options.SlangCompilerPath << '\n';
				return 1;
			}

			for (fs::path& includeDirectory : options.IncludeDirectories)
			{
				includeDirectory = fs::weakly_canonical(includeDirectory);
			}

			const fs::path outputRoot = ResolveOutputRoot(options);
			const fs::path cachePath = options.CachePath.empty() ? (outputRoot / ".shadercompiler-cache.json") : options.CachePath;
			Manifest manifest = LoadManifest(cachePath);
			const std::vector<fs::path> shaderFiles = CollectShaderFiles(options.SourcePath);

			if (shaderFiles.empty())
			{
				std::cout << "No .slang files found under " << options.SourcePath << '\n';
				return 0;
			}

			size_t totalEntryCount = 0;
			size_t dirtyEntryCount = 0;
			size_t compiledEntryCount = 0;

			for (const fs::path& sourceFile : shaderFiles)
			{
				const std::string sourceHash = HashFileContents(sourceFile);
				const std::string sourceWriteTimeUtc = FileTimeToUtcString(fs::last_write_time(sourceFile));
				const std::vector<EntryPointInfo> entryPoints = ResolveEntryPoints(options, sourceFile);

				for (const EntryPointInfo& entryPoint : entryPoints)
				{
					++totalEntryCount;
					const fs::path outputFile = ResolveOutputPath(options, outputRoot, sourceFile, entryPoint);
					const std::string recordKey = NormalizePathString(sourceFile) + "::" + entryPoint.Name;
					const bool needsRecompile = NeedsRecompile(options, manifest, recordKey, outputFile, entryPoint, sourceHash, sourceWriteTimeUtc);

					if (!needsRecompile)
					{
						if (options.Verbose)
						{
							std::cout << "[skip] " << sourceFile << " (" << entryPoint.Name << ")\n";
						}
						continue;
					}

					++dirtyEntryCount;
					if (options.CheckOnly)
					{
						std::cout << "[stale] " << sourceFile << " (" << entryPoint.Name << ")\n";
						continue;
					}

					if (CompileEntryPoint(options, manifest, recordKey, sourceFile, outputFile, entryPoint, sourceHash, sourceWriteTimeUtc) != 0)
					{
						return 1;
					}
					++compiledEntryCount;
				}
			}

			if (options.CheckOnly)
			{
				if (dirtyEntryCount == 0)
				{
					std::cout << "All shaders are up to date.\n";
					return 0;
				}

				std::cout << dirtyEntryCount << " shader entry point(s) are stale or missing compiled outputs.\n";
				return 2;
			}

			SaveManifest(cachePath, manifest);
			std::cout << "Compiled " << compiledEntryCount << " shader entry point(s); " << (totalEntryCount - compiledEntryCount) << " up to date.\n";
			return 0;
		}
		catch (const std::exception& exception)
		{
			std::cerr << exception.what() << '\n';
			return 1;
		}
	}
}

int main(int argc, char** argv)
{
	return ShaderCompiler::Run(argc, argv);
}
