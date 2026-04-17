#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"
#include "Engine/Assets/AssimpModelLoader.h"

#include <string>
#include <vector>

namespace Engine
{
    // -----------------------------------------------------------------------
    // Model — top-level runtime asset produced by ModelFactory.
    // Owns one Mesh (all submeshes share a single VB/IB) and the
    // raw material import data until the Material system is ready to
    // consume it.
    // -----------------------------------------------------------------------
    struct Model
    {
        std::string                     Name;
        Ref<Mesh>                       MeshData;
        std::vector<MaterialImportData> Materials;   // consumed by Material Factory later
    };

    // -----------------------------------------------------------------------
    // ModelFactory
    //
    // Converts a ModelImportResult (raw Assimp output) into a Model ready
    // for the ECS / SceneRenderer.
    //
    // All MeshImportData entries are merged into a single Mesh with one
    // SubMesh per entry, sharing a combined VertexBuffer and IndexBuffer.
    // This keeps draw call setup simple and avoids per-mesh GPU buffer
    // allocation for typical static models.
    //
    // Usage:
    //   auto result = AssimpModelLoader::Load("assets/models/helmet.gltf");
    //   if (result)
    //   {
    //       Model model = ModelFactory::BuildFromImport("helmet", result);
    //       // model.MeshData is GPU-ready
    //   }
    // -----------------------------------------------------------------------
    class ModelFactory
    {
    public:
        ModelFactory()  = delete;
        ~ModelFactory() = delete;

        // Build a Model from a ModelImportResult.
        // Calls Mesh::UploadToGPU() internally — must be called on the
        // render thread (or before first use in the render loop).
        [[nodiscard]] static Model BuildFromImport(const std::string& name, const ModelImportResult& importResult);

        // load + build in one call.
        [[nodiscard]] static Model Load(const std::string& filepath, const ModelLoadOptions& options = {});

    private:
        static Ref<Mesh> MergeMeshImports(const std::vector<MeshImportData>& meshes);
    };

} // namespace Kairos
