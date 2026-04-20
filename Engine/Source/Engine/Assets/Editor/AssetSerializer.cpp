#include "kepch.h"
#include "AssetSerializer.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>

namespace Engine
{
	// -----------------------------------------------------------------------
	// Internal helpers
	// -----------------------------------------------------------------------
	static std::string AssetTypeToString(AssetType type)
	{
		switch (type)
		{
			case AssetType::Mesh:     return "Mesh";
			case AssetType::Texture:  return "Texture";
			case AssetType::Material: return "Material";
			case AssetType::Shader:   return "Shader";
			case AssetType::Scene:    return "Scene";
			default:                  return "None";
		}
	}

	static AssetType AssetTypeFromString(const std::string& s)
	{
		if (s == "Mesh")     return AssetType::Mesh;
		if (s == "Texture")  return AssetType::Texture;
		if (s == "Material") return AssetType::Material;
		if (s == "Shader")   return AssetType::Shader;
		if (s == "Scene")    return AssetType::Scene;
		return AssetType::None;
	}

	// FNV-1a 64-bit hash — no external deps, good enough for change detection
	static std::string FNV1aHash(const std::filesystem::path& path)
	{
		std::ifstream f(path, std::ios::binary);
		if (!f) return "";

		constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
		constexpr uint64_t FNV_PRIME  = 1099511628211ULL;
		uint64_t hash = FNV_OFFSET;

		char buf[8192];
		while (f.read(buf, sizeof(buf)) || f.gcount() > 0)
		{
			for (std::streamsize i = 0; i < f.gcount(); ++i)
			{
				hash ^= static_cast<uint8_t>(buf[i]);
				hash *= FNV_PRIME;
			}
		}

		std::ostringstream oss;
		oss << std::hex << hash;
		return oss.str();
	}

	// -----------------------------------------------------------------------
	// AssetSerializer
	// -----------------------------------------------------------------------
	std::filesystem::path AssetSerializer::GetKassetPath(const std::filesystem::path& sourcePath)
	{
		return std::filesystem::path(sourcePath.string() + ".kasset");
	}

	std::string AssetSerializer::ComputeSourceHash(const std::filesystem::path& sourcePath)
	{
		return FNV1aHash(sourcePath);
	}

	bool AssetSerializer::Write(const std::filesystem::path& sourcePath, const AssetMetadata& metadata)
	{
		std::filesystem::path kassetPath = GetKassetPath(sourcePath);

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Asset" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Handle"          << YAML::Value << static_cast<uint64_t>(metadata.Handle);
		out << YAML::Key << "Type"            << YAML::Value << AssetTypeToString(metadata.Type);
		out << YAML::Key << "Version"         << YAML::Value << 1;
		out << YAML::Key << "ImporterVersion" << YAML::Value << CurrentImporterVersion;
		out << YAML::EndMap;

		out << YAML::Key << "Source" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "File" << YAML::Value << sourcePath.filename().string();
		out << YAML::Key << "Hash" << YAML::Value << ComputeSourceHash(sourcePath);
		out << YAML::EndMap;

		out << YAML::Key << "ImportSettings" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Scale"   << YAML::Value << 1.0;
		out << YAML::Key << "FlipUVs" << YAML::Value << false;
		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream file(kassetPath);
		if (!file)
		{
			LOG(LogLevel::Error, "AssetSerializer: failed to write '{}'.", kassetPath.string());
			return false;
		}
		file << out.c_str();
		return true;
	}

	AssetMetadata AssetSerializer::Read(const std::filesystem::path& kassetPath)
	{
		AssetMetadata metadata;

		YAML::Node root;
		try { root = YAML::LoadFile(kassetPath.string()); }
		catch (const YAML::Exception& e)
		{
			LOG(LogLevel::Error, "AssetSerializer: parse error '{}': {}", kassetPath.string(), e.what());
			return metadata;
		}

		auto asset = root["Asset"];
		if (!asset) return metadata;

		metadata.Handle          = AssetHandle(asset["Handle"].as<uint64_t>(0));
		metadata.Type            = AssetTypeFromString(asset["Type"].as<std::string>("None"));
		metadata.ImporterVersion = asset["ImporterVersion"].as<uint32_t>(0);

		if (auto source = root["Source"])
			metadata.SourceHash = source["Hash"].as<std::string>("");

		return metadata;
	}

} // namespace Engine
