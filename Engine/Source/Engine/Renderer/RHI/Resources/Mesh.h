#pragma once

#include "Engine/Renderer/RHI/Buffer.h"

namespace Engine
{
	class Mesh
	{
	public:
		Mesh(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) : m_VertexBuffer(vb), m_IndexBuffer(ib), m_Layout(vb->GetLayout()) {}

		const BufferLayout& GetLayout() const { return m_Layout; }

	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer>  m_IndexBuffer;
		BufferLayout      m_Layout;
	};
}
