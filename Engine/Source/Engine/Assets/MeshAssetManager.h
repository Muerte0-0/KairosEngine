#pragma once
#include "Engine/Debugging/Log.h"
#include "Engine/Project/Project.h"
#include "Engine/Renderer/RHI/ModelFactory.h"

namespace Engine
{
	class MeshAssetManager
	{
	public:
		static bool IsMeshAssetPath(const std::filesystem::path& path)
		{
			std::string extension = path.extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char character) { return static_cast<char>(std::tolower(character)); });

			static constexpr std::array SupportedExtensions = { ".fbx", ".obj", ".gltf", ".glb" };
			return std::find(SupportedExtensions.begin(), SupportedExtensions.end(), extension) != SupportedExtensions.end();
		}

		static Ref<Mesh> GetMesh(const std::filesystem::path& path)
		{
			if (path.empty())
				return nullptr;

			if (!IsMeshAssetPath(path))
			{
				LOG(LogLevel::Warning, "MeshAssetManager: '{}' is not supported mesh asset.", path.string());
				return nullptr;
			}
			
			if (!std::filesystem::exists(path))
			{
				LOG(LogLevel::Warning, "MeshAssetManager: Mesh asset '{}' does not exist.", path.string());
				return nullptr;
			}

			std::string cacheKey = path.generic_string();
			auto& cache = GetCache();
			if (auto iterator = cache.find(cacheKey); iterator != cache.end())
				return iterator->second;

			Model model = ModelFactory::Load(path.string());
			if (!model.MeshData)
			{
				LOG(LogLevel::Error, "MeshAssetManager: Failed to load mesh asset '{}'.", path.string());
				return nullptr;
			}

			cache.emplace(cacheKey, model.MeshData);
			return model.MeshData;
		}

		static std::string GetDisplayName(const std::filesystem::path& path)
		{
			if (path.empty())
				return "None";

			return path.filename().string();
		}

	private:
		static std::unordered_map<std::string, Ref<Mesh>>& GetCache()
		{
			static std::unordered_map<std::string, Ref<Mesh>> s_MeshCache;
			return s_MeshCache;
		}
	};
}
