#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "Engine/Debugging/Log.h"
#include "Engine/Renderer/RHI/ModelFactory.h"

namespace Engine
{
	class MeshAssetManager
	{
	public:
		static const std::filesystem::path& GetContentRoot()
		{
			static const std::filesystem::path s_ContentRoot = "Content";
			return s_ContentRoot;
		}

		static std::filesystem::path NormalizeAssetPath(const std::filesystem::path& path)
		{
			if (path.empty())
				return {};

			std::filesystem::path normalizedPath = path.lexically_normal();
			const std::filesystem::path& contentRoot = GetContentRoot();

			if (normalizedPath.is_absolute())
			{
				const std::filesystem::path absoluteContentRoot = std::filesystem::absolute(contentRoot).lexically_normal();
				std::error_code errorCode;
				std::filesystem::path relativePath = std::filesystem::relative(normalizedPath, absoluteContentRoot, errorCode);
				if (!errorCode && !relativePath.empty())
					normalizedPath = relativePath.lexically_normal();
			}

			return normalizedPath;
		}

		static std::filesystem::path GetAbsoluteAssetPath(const std::filesystem::path& path)
		{
			std::filesystem::path normalizedPath = NormalizeAssetPath(path);
			if (normalizedPath.empty())
				return {};

			return std::filesystem::absolute(GetContentRoot() / normalizedPath).lexically_normal();
		}

		static bool IsMeshAssetPath(const std::filesystem::path& path)
		{
			std::string extension = path.extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char character) { return static_cast<char>(std::tolower(character)); });

			static constexpr std::array SupportedExtensions = { ".fbx", ".obj", ".gltf", ".glb" };
			return std::find(SupportedExtensions.begin(), SupportedExtensions.end(), extension) != SupportedExtensions.end();
		}

		static Ref<Mesh> GetMesh(const std::filesystem::path& relativePath)
		{
			std::filesystem::path normalizedPath = NormalizeAssetPath(relativePath);
			if (normalizedPath.empty())
				return nullptr;

			if (!IsMeshAssetPath(normalizedPath))
			{
				LOG(LogLevel::Warning, "MeshAssetManager: '{}' is not supported mesh asset.", normalizedPath.string());
				return nullptr;
			}

			std::filesystem::path absolutePath = GetAbsoluteAssetPath(normalizedPath);
			if (!std::filesystem::exists(absolutePath))
			{
				LOG(LogLevel::Warning, "MeshAssetManager: Mesh asset '{}' does not exist.", absolutePath.string());
				return nullptr;
			}

			std::string cacheKey = normalizedPath.generic_string();
			auto& cache = GetCache();
			if (auto iterator = cache.find(cacheKey); iterator != cache.end())
				return iterator->second;

			Model model = ModelFactory::Load(absolutePath.string());
			if (!model.MeshData)
			{
				LOG(LogLevel::Error, "MeshAssetManager: Failed to load mesh asset '{}'.", absolutePath.string());
				return nullptr;
			}

			cache.emplace(cacheKey, model.MeshData);
			return model.MeshData;
		}

		static std::string GetDisplayName(const std::filesystem::path& path)
		{
			std::filesystem::path normalizedPath = NormalizeAssetPath(path);
			if (normalizedPath.empty())
				return "None";

			return normalizedPath.filename().string();
		}

	private:
		static std::unordered_map<std::string, Ref<Mesh>>& GetCache()
		{
			static std::unordered_map<std::string, Ref<Mesh>> s_MeshCache;
			return s_MeshCache;
		}
	};
}
