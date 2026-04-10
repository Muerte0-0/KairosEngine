#include "kepch.h"
#include "Engine/Renderer/RHI/AssimpModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <filesystem>

namespace Engine
{
    // -----------------------------------------------------------------------
    // Public entry point
    // -----------------------------------------------------------------------
    ModelImportResult AssimpModelLoader::Load(const std::string& filepath, const ModelLoadOptions& options)
    {
        ModelImportResult result;

        // --- Build Assimp post-process flags ---
        // NOTE: aiProcess_ValidateDataStructure is intentionally excluded.
        // It incorrectly rejects valid FBX files whose UV transform properties
        // are stored as matrices (40 bytes) rather than the 5-float vector (20 bytes)
        // that the validator expects. This is an Assimp quirk, not a broken file.
        uint32_t flags =
            aiProcess_Triangulate           |   // all faces → triangles
            aiProcess_JoinIdenticalVertices |   // deduplicate verts
            aiProcess_ImproveCacheLocality;     // reorder for GPU cache

        if (options.FlipUVs)
            flags |= aiProcess_FlipUVs;

        if (options.GenerateNormals)
            flags |= aiProcess_GenSmoothNormals;

        if (options.CalcTangents)
            flags |= aiProcess_CalcTangentSpace;

        if (options.OptimizeMeshes)
            flags |= aiProcess_OptimizeMeshes;

        if (options.MergeMeshes)
            flags |= aiProcess_OptimizeGraph;

        // --- Import ---
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath, flags);

        if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
        {
            result.Success      = false;
            result.ErrorMessage = importer.GetErrorString();
            LOG(LogLevel::Error, "AssimpModelLoader: Failed to load '{}' — {}", filepath, result.ErrorMessage);
            return result;
        }

        // Extract materials first — meshes reference them by index
        std::string directory = std::filesystem::path(filepath).parent_path().string();
        result.Materials = ExtractMaterials(scene, directory);

        // Traverse node hierarchy and collect meshes
        ProcessNode(scene->mRootNode, scene, result);

        result.Success = true;
        LOG(LogLevel::Info, "AssimpModelLoader: Loaded '{}' — {} mesh(es), {} material(s)",
            filepath,
            result.Meshes.size(),
            result.Materials.size());

        return result;
    }

    // -----------------------------------------------------------------------
    // Node traversal — recursive
    // -----------------------------------------------------------------------
    void AssimpModelLoader::ProcessNode(const aiNode* node, const aiScene* scene, ModelImportResult& out)
    {
        for (uint32_t i = 0; i < node->mNumMeshes; ++i)
        {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            out.Meshes.push_back(ProcessMesh(mesh, scene));
        }

        for (uint32_t i = 0; i < node->mNumChildren; ++i)
            ProcessNode(node->mChildren[i], scene, out);
    }

    // -----------------------------------------------------------------------
    // Per-mesh extraction
    // -----------------------------------------------------------------------
    MeshImportData AssimpModelLoader::ProcessMesh(const aiMesh* mesh, const aiScene* /*scene*/)
    {
        MeshImportData data;
        data.Name          = mesh->mName.C_Str();
        data.MaterialIndex = mesh->mMaterialIndex;

        data.Vertices.reserve(mesh->mNumVertices);
        data.Indices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3);

        // ---- Vertices ----
        for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex v{};

            // Position — always present after Triangulate
            v.Position = {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            };

            // Normal — present after GenSmoothNormals
            if (mesh->HasNormals())
            {
                v.Normal = {
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                };
            }

            // Tangent + Bitangent — present after CalcTangentSpace (requires UVs)
            if (mesh->HasTangentsAndBitangents())
            {
                v.Tangent = {
                    mesh->mTangents[i].x,
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z
                };
                v.Bitangent = {
                    mesh->mBitangents[i].x,
                    mesh->mBitangents[i].y,
                    mesh->mBitangents[i].z
                };
            }

            // UV channel 0 — most models only have one set
            if (mesh->HasTextureCoords(0))
            {
                v.TexCoord = {
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                };
            }

            data.Vertices.push_back(v);
        }

        // ---- Indices ----
        // Faces are guaranteed to be triangles after aiProcess_Triangulate
        for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            ASSERT(face.mNumIndices == 3, "AssimpModelLoader: Non-triangle face encountered after Triangulate!");

            data.Indices.push_back(face.mIndices[0]);
            data.Indices.push_back(face.mIndices[1]);
            data.Indices.push_back(face.mIndices[2]);
        }

        return data;
    }

    // -----------------------------------------------------------------------
    // Material extraction — one MaterialImportData per aiMaterial slot
    // -----------------------------------------------------------------------
    std::vector<MaterialImportData> AssimpModelLoader::ExtractMaterials(const aiScene* scene, const std::string& modelDirectory)
    {
        std::vector<MaterialImportData> materials;
        materials.reserve(scene->mNumMaterials);

        for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
        {
            const aiMaterial* aiMat = scene->mMaterials[i];
            MaterialImportData mat;

            // Name
            aiString name;
            if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
                mat.Name = name.C_Str();
            else
                mat.Name = "Material_" + std::to_string(i);

            // Base colour (PBR albedo or classic diffuse — whichever is present)
            aiColor4D colour;
            if (aiMat->Get(AI_MATKEY_BASE_COLOR, colour) == AI_SUCCESS ||
                aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, colour) == AI_SUCCESS)
            {
                mat.AlbedoColor = { colour.r, colour.g, colour.b, colour.a };
            }

            // Metallic / Roughness (PBR — glTF 2.0 and FBX PBR)
            float metallic = 0.f, roughness = 1.f;
            aiMat->Get(AI_MATKEY_METALLIC_FACTOR,   metallic);
            aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR,  roughness);
            mat.Metallic   = metallic;
            mat.Roughness  = roughness;

            // Texture paths
            mat.AlbedoTexturePath        = ExtractTexturePath(aiMat, aiTextureType_BASE_COLOR,          modelDirectory);
            if (mat.AlbedoTexturePath.empty())  // fallback for older formats
                mat.AlbedoTexturePath    = ExtractTexturePath(aiMat, aiTextureType_DIFFUSE,              modelDirectory);

            mat.NormalTexturePath        = ExtractTexturePath(aiMat, aiTextureType_NORMALS,              modelDirectory);
            if (mat.NormalTexturePath.empty())
                mat.NormalTexturePath    = ExtractTexturePath(aiMat, aiTextureType_HEIGHT,               modelDirectory);

            mat.MetallicRoughnessPath    = ExtractTexturePath(aiMat, aiTextureType_DIFFUSE_ROUGHNESS,    modelDirectory);
            mat.AOTexturePath            = ExtractTexturePath(aiMat, aiTextureType_AMBIENT_OCCLUSION,    modelDirectory);
            mat.EmissiveTexturePath      = ExtractTexturePath(aiMat, aiTextureType_EMISSIVE,             modelDirectory);

            materials.push_back(std::move(mat));
        }

        return materials;
    }

    // -----------------------------------------------------------------------
    // Texture path helper
    // Returns the resolved absolute path, or "" if the slot is absent.
    // -----------------------------------------------------------------------
    std::string AssimpModelLoader::ExtractTexturePath(const aiMaterial* mat, int textureType, const std::string& modelDirectory)
    {
        aiString path;
        if (mat->GetTexture(static_cast<aiTextureType>(textureType), 0, &path) != AI_SUCCESS)
            return "";

        // Assimp may give us an absolute path, a relative path, or an embedded
        // texture token (*0, *1, …). For now just resolve relative paths.
        std::filesystem::path texPath(path.C_Str());

        if (texPath.is_absolute())
            return texPath.string();

        // Relative — join with the directory the model file lives in
        return (std::filesystem::path(modelDirectory) / texPath).string();
    }

}
