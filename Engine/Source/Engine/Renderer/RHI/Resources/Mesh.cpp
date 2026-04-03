#include "kepch.h"
#include "Mesh.h"

namespace Engine
{
	Mesh::Mesh(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib): m_VertexBuffer(vb), m_IndexBuffer(ib), m_Layout(vb->GetLayout())
	{
		
	}
}
