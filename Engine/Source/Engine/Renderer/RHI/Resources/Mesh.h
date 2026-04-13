#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Renderer/RHI/Buffer.h"
#include "Engine/Utils/RendererUtils.h"

namespace Engine
{
    // -----------------------------------------------------------------------
    // Canonical vertex layout — must match Mesh.slang input struct exactly.
    // Used by VulkanGraphicsPipeline to build vertex input state.
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
    // Lifecycle:
    //   Option A (procedural / hand-built):
    //     auto mesh = Mesh::Create(vertices, indices);   // single submesh
    //
    //   Option B (multi-submesh via ModelFactory):
    //     auto mesh = CreateRef<Mesh>();
    //     mesh->Populate(vertices, indices, subMeshes);
    //     mesh->UploadToGPU();
    //
    // Ownership: Ref<Mesh> (shared_ptr). GPU buffers destroyed with the Mesh.
    // -----------------------------------------------------------------------
    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh() = default;

        Mesh(const Mesh&)            = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&)                 = default;
        Mesh& operator=(Mesh&&)      = default;

        // ---- Factory (single submesh shortcut) ----------------------------

        // Creates a Mesh from raw arrays, sets up a single SubMesh covering
        // all geometry, calls UploadToGPU(), and returns a Ref<Mesh>.
        [[nodiscard]] static Ref<Mesh> Create(
            std::vector<Vertex>   vertices,
            std::vector<uint32_t> indices);

        // ---- Multi-submesh population (used by ModelFactory) --------------

        // Replaces all CPU-side data and builds SubMeshes list.
        // Call UploadToGPU() afterwards.
        void Populate(
            std::vector<Vertex>   vertices,
            std::vector<uint32_t> indices,
            std::vector<SubMesh>  subMeshes);

        // ---- GPU upload ---------------------------------------------------

        // Uploads Vertices + Indices to GPU VertexBuffer / IndexBuffer.
        // Sets layout on the VertexBuffer to GetVertexLayout().
        // Safe to call more than once — re-creates buffers each time.
        // Must be called after Populate() or Create().
        void UploadToGPU();

        // ---- Const accessors ----------------------------------------------

        [[nodiscard]] const std::vector<Vertex>&   GetVertices()    const { return m_Vertices;  }
        [[nodiscard]] const std::vector<uint32_t>& GetIndices()     const { return m_Indices;   }
        [[nodiscard]] const std::vector<SubMesh>&  GetSubMeshes()   const { return m_SubMeshes; }
        [[nodiscard]] const BufferLayout&          GetLayout()      const { return m_Layout;    }

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
        std::vector<Vertex>   m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<SubMesh>  m_SubMeshes;
        BufferLayout          m_Layout;

        Ref<VertexBuffer>     m_VertexBuffer;
        Ref<IndexBuffer>      m_IndexBuffer;
    };

} // namespace Engine
