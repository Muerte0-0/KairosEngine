#include "kepch.h"
#include "Mesh.h"

#include "Engine/Renderer/RHI/Buffer.h"

namespace Engine
{
    // -----------------------------------------------------------------------
    // Mesh::Create — single-submesh convenience factory
    // -----------------------------------------------------------------------
    Ref<Mesh> Mesh::Create(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    {
        auto mesh = CreateRef<Mesh>();

        // Build a single SubMesh that covers all geometry
        SubMesh sub;
        sub.Name        = "Default";
        sub.BaseVertex  = 0;
        sub.BaseIndex   = 0;
        sub.IndexCount  = static_cast<uint32_t>(indices.size());
        sub.MaterialIndex = 0;

        mesh->m_Vertices  = std::move(vertices);
        mesh->m_Indices   = std::move(indices);
        mesh->m_SubMeshes = { sub };
        mesh->m_Layout    = GetVertexLayout();

        mesh->UploadToGPU();
        return mesh;
    }

    // -----------------------------------------------------------------------
    // Mesh::Populate — multi-submesh data assignment (used by ModelFactory)
    // -----------------------------------------------------------------------
    void Mesh::Populate(
        std::vector<Vertex>   vertices,
        std::vector<uint32_t> indices,
        std::vector<SubMesh>  subMeshes)
    {
        m_Vertices  = std::move(vertices);
        m_Indices   = std::move(indices);
        m_SubMeshes = std::move(subMeshes);
        m_Layout    = GetVertexLayout();
    }

    // -----------------------------------------------------------------------
    // Mesh::UploadToGPU
    // -----------------------------------------------------------------------
    void Mesh::UploadToGPU()
    {
        ASSERT(!m_Vertices.empty(), "Mesh::UploadToGPU — no vertex data to upload.");
        ASSERT(!m_Indices.empty(),  "Mesh::UploadToGPU — no index data to upload.");

        const uint32_t vertexDataSize = static_cast<uint32_t>(m_Vertices.size() * sizeof(Vertex));
        const uint32_t indexCount     = static_cast<uint32_t>(m_Indices.size());

        // Create (or re-create) the vertex buffer
        m_VertexBuffer = VertexBuffer::Create(
            m_Vertices.data(),
            vertexDataSize,
            BufferUsage::Static);

        m_VertexBuffer->SetLayout(m_Layout);

        // Create (or re-create) the index buffer
        m_IndexBuffer = IndexBuffer::Create(
            m_Indices.data(),
            indexCount,
            sizeof(uint32_t),
            BufferUsage::Static);
    }

} // namespace Engine
