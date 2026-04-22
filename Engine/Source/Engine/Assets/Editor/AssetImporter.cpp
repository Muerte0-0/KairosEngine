#include "kepch.h"
#include "AssetImporter.h"
#include "Engine/Project/Project.h"
#include "Engine/Renderer/RHI/Factories/ModelFactory.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"
#include "Engine/Renderer/RHI/Resources/Texture.h"
#include "Engine/Materials/MaterialGraph.h"

namespace Engine
{
	AssetType AssetImporter::DeduceTypeFromPath(const std::filesystem::path& path)
	{
		static const std::unordered_map<std::string, AssetType> s_ExtMap =
		{
			{ ".fbx",  AssetType::Mesh     },
			{ ".obj",  AssetType::Mesh     },
			{ ".gltf", AssetType::Mesh     },
			{ ".glb",  AssetType::Mesh     },
			{ ".png",  AssetType::Texture  },
			{ ".jpg",  AssetType::Texture  },
			{ ".jpeg", AssetType::Texture  },
			{ ".tga",  AssetType::Texture  },
			{ ".ktx",  AssetType::Texture  },
			{ ".kmat", AssetType::Material },
			{ ".ksh",  AssetType::Shader   },
			{ ".kscn", AssetType::Scene    },
		};

		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		auto it = s_ExtMap.find(ext);
		return it != s_ExtMap.end() ? it->second : AssetType::None;
	}

	Ref<Asset> AssetImporter::Import(const AssetMetadata& metadata)
	{
		switch (metadata.Type)
		{
			case AssetType::Mesh:     return ImportMesh(metadata);
			case AssetType::Texture:  return ImportTexture(metadata);
			case AssetType::Material: return ImportMaterial(metadata);
			default:
				LOG(LogLevel::Error, "AssetImporter: no importer for AssetType '{}'.", (uint16_t)metadata.Type);
				return nullptr;
		}
	}

	Ref<Asset> AssetImporter::ImportMesh(const AssetMetadata& metadata)
	{
		std::filesystem::path absolutePath = Project::GetAssetPath(metadata.FilePath);

		if (!std::filesystem::exists(absolutePath))
		{
			LOG(LogLevel::Error, "AssetImporter: mesh file not found: '{}'.", absolutePath.string());
			return nullptr;
		}

		Model model = ModelFactory::Load(absolutePath.string());
		if (!model.MeshData)
		{
			LOG(LogLevel::Error, "AssetImporter: failed to load mesh: '{}'.", absolutePath.string());
			return nullptr;
		}

		// Store materials on the mesh so consumers can retrieve them via GetMaterials().
		model.MeshData->SetMaterials(std::move(model.Materials));
		model.MeshData->Handle = metadata.Handle;
		return model.MeshData;
	}

	Ref<Asset> AssetImporter::ImportTexture(const AssetMetadata& metadata)
	{
		std::filesystem::path absolutePath = Project::GetAssetPath(metadata.FilePath);

		if (!std::filesystem::exists(absolutePath))
		{
			LOG(LogLevel::Error, "AssetImporter: texture file not found: '{}'.", absolutePath.string());
			return nullptr;
		}

		Ref<Texture> texture = Texture::Create(absolutePath);
		if (!texture)
		{
			LOG(LogLevel::Error, "AssetImporter: failed to load texture: '{}'.", absolutePath.string());
			return nullptr;
		}

		texture->Handle = metadata.Handle;
		return texture;
	}

	Ref<Asset> AssetImporter::ImportMaterial(const AssetMetadata& metadata)
	{
		std::filesystem::path absolutePath = Project::GetAssetPath(metadata.FilePath);

		auto material = CreateRef<MaterialAsset>();
		material->Handle = metadata.Handle;

		// Treat missing OR empty files as brand-new — init default and write
		bool isEmpty = !std::filesystem::exists(absolutePath) ||
		               std::filesystem::file_size(absolutePath) == 0;

		if (isEmpty)
		{
			material->Graph.Name = metadata.FilePath.stem().string();
			material->Graph.InitDefault();
			material->Graph.SaveToFile(absolutePath);
		}
		else
		{
			if (!material->Graph.LoadFromFile(absolutePath))
			{
				LOG(LogLevel::Error, "AssetImporter: failed to parse material '{}'.", absolutePath.string());
				return nullptr;
			}
		}

		return material;
	}
}
