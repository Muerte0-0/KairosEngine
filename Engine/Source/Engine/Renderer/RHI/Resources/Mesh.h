#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Renderer/RHI/Buffer.h"
#include "Engine/Utils/RendererUtils.h"
#include "Engine/Assets/Asset.h"

// Forward-declare to avoid circular include (Material.h → Texture.h → ... → Mesh.h)
namespace Engine { class Material; }

namespace Engine
{
    // -----------------------------------------------------------------------
    // Canonical vertex layout — must match Mesh.slang input struct exactly.
    // -----------------------------------------------------------------------
    inline BufferLayout GetVertexLayout()
    {
        return {
            { ShaderDataType::Float3, "a_Position"  },
            { ShaderDataType::Float3, "a_Normal"    },
            { ShaderDataType::Float3, "a_Tangent"   },
            { ShaderDataType::Float3, "a_Bitangent" },
            { ShaderDataType::Float2, "a_TexCoord"  },
        };
    }

    // -----------------------------------------------------------------------
    // SubMesh — one indexed draw call inside a shared VB/IB pair.
    // MaterialIndex indexes into the owning Model's Materials array.
    // -----------------------------------------------------------------------
    struct SubMesh
    {
        std::string Name;
        uint32_t    BaseVertex    { 0 };
        uint32_t    BaseIndex     { 0 };
        uint32_t    IndexCount    { 0 };
        uint32_t    MaterialIndex { 0 };
    };

    // -----------------------------------------------------------------------
    // Mesh — CPU geometry + GPU buffers.
    //
    // Life cycle:
    //   Option A (procedural / hand-built):
    //     auto mesh = Mesh::Create(vertices, indices); // single SubMesh
    //
    //   Option B (Multi-SubMesh via Model Factory):
    //     auto mesh = CreateRef<Mesh>();
    //     mesh->Populate(vertices, indices, subMeshes);
    //     mesh->UploadToGPU();
    //
    // Ownership: Ref<Mesh> (shared_ptr). GPU buffers destroyed with the Mesh.
    // -----------------------------------------------------------------------
    class Mesh : public Asset
    {
    public:
        static AssetType GetStaticType() { return AssetType::Mesh; }
        AssetType        GetType() const override { return GetStaticType(); }

        Mesh() = default;
        ~Mesh() = default;

        Mesh(const Mesh&)            = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&)                 = default;
        Mesh& operator=(Mesh&&)      = default;
        
        // Creates a Mesh from raw arrays, sets up a single SubMesh covering
        // all geometry, calls UploadToGPU(), and returns a Ref<Mesh>.
        [[nodiscard]] static Ref<Mesh> Create(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
        
        // Replaces all CPU-side data and builds SubMeshes list.
        // Call UploadToGPU() afterwords.
        void Populate(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<SubMesh> subMeshes);
        
        void UploadToGPU();

        // ---- Const accessors ----------------------------------------------

        [[nodiscard]] const std::vector<Vertex>&        GetVertices()    const { return m_Vertices;  }
        [[nodiscard]] const std::vector<uint32_t>&       GetIndices()     const { return m_Indices;   }
        [[nodiscard]] const std::vector<SubMesh>&        GetSubMeshes()   const { return m_SubMeshes; }
        [[nodiscard]] const BufferLayout&                GetLayout()      const { return m_Layout;    }
        [[nodiscard]] const std::vector<Ref<Material>>&  GetMaterials()   const { return m_Materials; }
        void SetMaterials(std::vector<Ref<Material>> materials) { m_Materials = std::move(materials); }

        [[nodiscard]] Ref<VertexBuffer>  GetVertexBuffer() const { return m_VertexBuffer; }
        [[nodiscard]] Ref<IndexBuffer>   GetIndexBuffer()  const { return m_IndexBuffer;  }

        [[nodiscard]] bool IsUploaded() const { return m_VertexBuffer && m_IndexBuffer; }

        // ---- AABB (local space) ------------------------------------------
        // Computed from CPU vertex data — valid after Populate() / Create().
        struct AABB
        {
            glm::vec3 Min{  FLT_MAX,  FLT_MAX,  FLT_MAX };
            glm::vec3 Max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
            bool IsValid() const { return Min.x <= Max.x; }
        };

        [[nodiscard]] AABB ComputeAABB() const
        {
            AABB aabb;
            for (const Vertex& v : m_Vertices)
            {
                aabb.Min = glm::min(aabb.Min, v.Position);
                aabb.Max = glm::max(aabb.Max, v.Position);
            }
            return aabb;
        }

    private:
        std::vector<Vertex>        m_Vertices;
        std::vector<uint32_t>      m_Indices;
        std::vector<SubMesh>       m_SubMeshes;
        BufferLayout               m_Layout;

        // Imported materials — indexed by SubMesh::MaterialIndex.
        // Set by AssetImporter after load; empty for procedural meshes.
        std::vector<Ref<Material>> m_Materials;

        Ref<VertexBuffer>          m_VertexBuffer;
        Ref<IndexBuffer>           m_IndexBuffer;
    };

} // namespace Engine
