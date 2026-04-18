#pragma once
#include "Engine/Debugging/Log.h"
#include "Engine/Project/Project.h"
#include "Engine/Renderer/RHI/Factories/ModelFactory.h"

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

		// Returns the full Model (mesh + materials). Cached after first load.
		static const Model* GetModel(const std::filesystem::path& path)
		{
			if (path.empty())
				return nullptr;

			if (!IsMeshAssetPath(path))
			{
				LOG(LogLevel::Warning, "MeshAssetManager: '{}' is not a supported mesh asset.", path.string());
				return nullptr;
			}

			if (!std::filesystem::exists(path))
			{
				LOG(LogLevel::Warning, "MeshAssetManager: mesh asset '{}' does not exist.", path.string());
				return nullptr;
			}

			std::string cacheKey = path.generic_string();
			auto& cache = GetModelCache();
			auto it = cache.find(cacheKey);
			if (it != cache.end())
				return &it->second;

			Model model = ModelFactory::Load(path.string());
			if (!model.MeshData)
			{
				LOG(LogLevel::Error, "MeshAssetManager: failed to load mesh asset '{}'.", path.string());
				return nullptr;
			}

			auto [inserted, ok] = cache.emplace(cacheKey, std::move(model));
			return &inserted->second;
		}

		// Convenience — returns just the Mesh (no materials).
		static Ref<Mesh> GetMesh(const std::filesystem::path& path)
		{
			const Model* model = GetModel(path);
			return model ? model->MeshData : nullptr;
		}

		static std::string GetDisplayName(const std::filesystem::path& path)
		{
			return path.empty() ? "None" : path.filename().string();
		}

	private:
		static std::unordered_map<std::string, Model>& GetModelCache()
		{
			static std::unordered_map<std::string, Model> s_Cache;
			return s_Cache;
		}
	};
}
