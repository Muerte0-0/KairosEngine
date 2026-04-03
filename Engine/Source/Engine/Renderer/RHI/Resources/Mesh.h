#pragma once

#include "Engine/Renderer/RHI/Buffer.h"

namespace Engine
{
	class Mesh
	{
	public:
		Mesh(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib);

		const Ref<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }		
		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }		
		const BufferLayout& GetLayout() const { return m_Layout; }

	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer>  m_IndexBuffer;
		BufferLayout      m_Layout;
	};
}
