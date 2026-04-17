#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Utils/RendererUtils.h"      // Vertex, PushConstantObject, etc.
#include "Engine/Renderer/RHI/Resources/Mesh.h"  // SubMesh, GetVertexLayout

#include <string>
#include <vector>

// Forward declare Assimp types — keep Assimp headers out of engine-wide includes
struct aiScene;
struct aiMesh;
struct aiNode;
struct aiMaterial;

namespace Engine
{
    // -----------------------------------------------------------------------
    // Flat description of one material slot as parsed from the file.
    // No GPU resources here — Material System consumes this later.
    // -----------------------------------------------------------------------
    struct MaterialImportData
    {
        std::string Name;

        glm::vec4   AlbedoColor         { 1.f, 1.f, 1.f, 1.f };
        float       Metallic            { 0.f };
        float       Roughness           { 1.f };

        // Relative paths as stored in the source file — resolved by the asset system
        std::string AlbedoTexturePath;
        std::string NormalTexturePath;
        std::string MetallicRoughnessPath;
        std::string AOTexturePath;
        std::string EmissiveTexturePath;
    };

    // -----------------------------------------------------------------------
    // Raw CPU-side geometry for one mesh in the file.
    // -----------------------------------------------------------------------
    struct MeshImportData
    {
        std::string           Name;
        std::vector<Vertex>   Vertices;
        std::vector<uint32_t> Indices;
        uint32_t              MaterialIndex { 0 };
    };

    // -----------------------------------------------------------------------
    // Top-level result returned by AssimpModelLoader::Load().
    // Consume with ModelFactory (or directly in Scene::LoadModel).
    // -----------------------------------------------------------------------
    struct ModelImportResult
    {
        std::vector<MeshImportData>     Meshes;
        std::vector<MaterialImportData> Materials;

        bool        Success      { false };
        std::string ErrorMessage;

        // Convenience
        explicit operator bool() const { return Success; }
    };

    // -----------------------------------------------------------------------
    // Optional flags forwarded to Assimp — sane defaults baked in Load().
    // Expose only what callers realistically override.
    // -----------------------------------------------------------------------
    struct ModelLoadOptions
    {
        bool FlipUVs          { true  };   // required for Vulkan NDC
        bool GenerateNormals  { true  };   // aiProcess_GenSmoothNormals
        bool CalcTangents     { true  };   // aiProcess_CalcTangentSpace
        bool OptimizeMeshes   { false };   // aiProcess_OptimizeMeshes — slower, worth it for shipping
        bool MergeMeshes      { false };   // aiProcess_OptimizeGraph — loses per-mesh names
    };

    // -----------------------------------------------------------------------
    // AssimpModelLoader
    //
    // Thin Assimp wrapper. Reads a model file and outputs engine-native
    // structs. No GPU resources are created here — that is the job of
    // whatever consumes Model Import Result (e.g. Model Factory, Scene).
    //
    // Usage:
    //   auto result = AssimpModelLoader::Load("assets/models/sponza.obj");
    //   if (result) { /* result.Meshes, result.Materials */ }
    // -----------------------------------------------------------------------
    class AssimpModelLoader
    {
    public:
        AssimpModelLoader()  = delete;
        ~AssimpModelLoader() = delete;

        [[nodiscard]] static ModelImportResult Load(const std::string& filepath, const ModelLoadOptions& options = {});

    private:
        // Recursive node traversal
        static void ProcessNode(const aiNode* node,const aiScene* scene,ModelImportResult& out);

        // Per-mesh extraction
        static MeshImportData ProcessMesh(const aiMesh* mesh, const aiScene* scene);

        // Material slot extraction
        static std::vector<MaterialImportData> ExtractMaterials(const aiScene* scene, const std::string& modelDirectory);

        // Texture path helper — returns "" if slot absent
        static std::string ExtractTexturePath(const aiMaterial* mat, int textureType, const std::string& modelDirectory);
    };

}
