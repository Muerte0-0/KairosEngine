#include "kepch.h"
#include "ModelFactory.h"

#include <filesystem>

namespace Engine
{
    // -----------------------------------------------------------------------
    // ModelFactory::BuildFromImport
    // -----------------------------------------------------------------------
    Model ModelFactory::BuildFromImport(
        const std::string&       name,
        const ModelImportResult& importResult)
    {
        ASSERT(importResult.Success, "ModelFactory: Cannot build from a failed import!");

        Model model;
        model.Name      = name;
        model.Materials = importResult.Materials;
        model.MeshData  = MergeMeshImports(importResult.Meshes);

        return model;
    }

    // -----------------------------------------------------------------------
    // ModelFactory::Load — convenience one-liner
    // -----------------------------------------------------------------------
    Model ModelFactory::Load(const std::string& filepath, const ModelLoadOptions& options)
    {
        ModelImportResult result = AssimpModelLoader::Load(filepath, options);

        if (!result)
        {
            LOG(LogLevel::Error, "ModelFactory::Load failed for '{}'", filepath);
            return {};
        }

        std::string name = std::filesystem::path(filepath).stem().string();
        return BuildFromImport(name, result);
    }

    // -----------------------------------------------------------------------
    // ModelFactory::MergeMeshImports
    //
    // Concatenates all MeshImportData arrays into a single combined VB/IB.
    // Each source mesh becomes one SubMesh, recording its base offsets.
    // -----------------------------------------------------------------------
    Ref<Mesh> ModelFactory::MergeMeshImports(const std::vector<MeshImportData>& meshes)
    {
        std::vector<Vertex>   combinedVertices;
        std::vector<uint32_t> combinedIndices;
        std::vector<SubMesh>  subMeshes;

        uint32_t baseVertex = 0;
        uint32_t baseIndex  = 0;

        for (const MeshImportData& src : meshes)
        {
            combinedVertices.insert(combinedVertices.end(), src.Vertices.begin(), src.Vertices.end());
            combinedIndices.insert(combinedIndices.end(),   src.Indices.begin(),  src.Indices.end());

            SubMesh sub;
            sub.Name          = src.Name;
            sub.BaseVertex    = baseVertex;
            sub.BaseIndex     = baseIndex;
            sub.IndexCount    = static_cast<uint32_t>(src.Indices.size());
            sub.MaterialIndex = src.MaterialIndex;
            subMeshes.push_back(sub);

            baseVertex += static_cast<uint32_t>(src.Vertices.size());
            baseIndex  += static_cast<uint32_t>(src.Indices.size());
        }

        auto mesh = CreateRef<Mesh>();
        mesh->Populate(std::move(combinedVertices), std::move(combinedIndices), std::move(subMeshes));
        mesh->UploadToGPU();
        return mesh;
    }

} // namespace Engine
