#pragma once
#include "Engine/Renderer/RendererUtils.h"

namespace Engine
{
	enum class BufferUsage : uint8_t
	{
		Static,   // GPU-only (Fast, Staged)
		Dynamic   // CPU-updated
	};
	
	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement() = default;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Float:  return 1;
				case ShaderDataType::Float2: return 2;
				case ShaderDataType::Float3: return 3;
				case ShaderDataType::Float4: return 4;
				case ShaderDataType::Mat3:   return 3 * 3;
				case ShaderDataType::Mat4:   return 4 * 4;
				default: break;
			}

			ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};
	
	class BufferLayout
	{
	public:
		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement>& elements) : m_Elements(elements)
		{ CalculateOffsetsAndStride(); }

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }

		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;

			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};
	
	// -----------------------------------------------------------------------
	// Vertex Buffer
	// -----------------------------------------------------------------------

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;
		
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		virtual uint32_t GetSize() const = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		virtual BufferUsage GetUsage() const = 0;
		
		static Ref<VertexBuffer> Create(const void* data, uint32_t size, BufferUsage usage = BufferUsage::Static);
	};
	
	// -----------------------------------------------------------------------
	// Index Buffer
	// -----------------------------------------------------------------------

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;
		
		virtual uint32_t GetCount() const = 0;
		virtual uint32_t GetIndexSize() const = 0;
		
		virtual BufferUsage GetUsage() const = 0;
		
		static Ref<IndexBuffer> Create(const void* indices, uint32_t count, uint32_t indexSize = sizeof(uint32_t), BufferUsage usage = BufferUsage::Static);
	};
}
