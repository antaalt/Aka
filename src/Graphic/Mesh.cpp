#include <Aka/Graphic/Mesh.h>
#include <Aka/Graphic/GraphicBackend.h>

namespace aka {

uint32_t size(IndexFormat format)
{
	switch (format)
	{
	case IndexFormat::UnsignedByte: return 1;
	case IndexFormat::UnsignedShort: return 2;
	case IndexFormat::UnsignedInt: return 4;
	default: return 0;
	}
}

uint32_t size(VertexFormat format)
{
	switch (format)
	{
	case VertexFormat::Float: return 4;
	case VertexFormat::Double: return 8;
	case VertexFormat::Byte: return 1;
	case VertexFormat::UnsignedByte: return 1;
	case VertexFormat::Short: return 2;
	case VertexFormat::UnsignedShort: return 2;
	case VertexFormat::Int: return 4;
	case VertexFormat::UnsignedInt: return 4;
	default: return 0;
	}
}

uint32_t size(VertexType type)
{
	switch (type)
	{
	case VertexType::Vec2: return 2;
	case VertexType::Vec3: return 3;
	case VertexType::Vec4: return 4;
	case VertexType::Mat2: return 4;
	case VertexType::Mat3: return 9;
	case VertexType::Mat4: return 16;
	case VertexType::Scalar: return 1;
	default: return 0;
	}
}

Mesh::Mesh() :
	m_indexAccessor{},
	m_vertexAccessors{}
{
}

Mesh::~Mesh()
{
}

Mesh::Ptr Mesh::create()
{
	return GraphicBackend::createMesh();
}

void Mesh::uploadInterleaved(const VertexAttribute* attributes, size_t attributeCount, void* vertices, uint32_t vertexCount)
{
	std::vector<VertexAccessor> info{};
	uint32_t stride = 0;
	for (uint32_t i = 0; i < attributeCount; i++)
		stride += attributes[i].size();
	VertexBufferView buffer{};
	buffer.buffer = Buffer::create(BufferType::VertexBuffer, vertexCount * stride, BufferUsage::Immutable, BufferCPUAccess::None, vertices);
	buffer.size = vertexCount * stride;
	buffer.offset = 0;
	buffer.stride = stride;
	uint32_t offset = 0;
	for (uint32_t i = 0; i < attributeCount; i++)
	{
		info.emplace_back(VertexAccessor{ attributes[i], buffer, offset, vertexCount });
		offset += attributes[i].size();
	}
	upload(info.data(), info.size());
}

void Mesh::uploadInterleaved(const VertexAttribute* attributes, size_t attributeCount, void* vertices, uint32_t vertexCount, IndexFormat indexFormat, void* indices, uint32_t indexCount)
{
	std::vector<VertexAccessor> vertexInfo{};
	uint32_t stride = 0;
	for (uint32_t i = 0; i < attributeCount; i++)
		stride += attributes[i].size();
	VertexBufferView buffer{};
	buffer.buffer = Buffer::create(BufferType::VertexBuffer, vertexCount * stride, BufferUsage::Immutable, BufferCPUAccess::None, vertices);
	buffer.size = vertexCount * stride;
	buffer.offset = 0;
	buffer.stride = stride;
	uint32_t offset = 0;
	for (uint32_t i = 0; i < attributeCount; i++)
	{
		vertexInfo.emplace_back(VertexAccessor{ attributes[i], buffer, offset, vertexCount });
		offset += attributes[i].size();
	}
	IndexAccessor indexInfo{};
	indexInfo.format = IndexFormat::UnsignedInt;
	indexInfo.bufferView.buffer = Buffer::create(BufferType::IndexBuffer, indexCount * 4, BufferUsage::Immutable, BufferCPUAccess::None, indices);
	indexInfo.bufferView.offset = 0;
	indexInfo.bufferView.size = indexCount * 4;
	upload(vertexInfo.data(), vertexInfo.size(), indexInfo);
}

bool Mesh::isIndexed() const
{
	return m_indexAccessor.bufferView.buffer != nullptr;
}

uint32_t Mesh::getIndexCount() const
{
	return m_indexAccessor.count;
}

IndexFormat Mesh::getIndexFormat() const
{
    return m_indexAccessor.format;
}

const IndexBufferView& Mesh::getIndexBuffer() const
{
	return m_indexAccessor.bufferView;
}

uint32_t Mesh::getVertexAttributeCount() const
{
	return static_cast<uint32_t>(m_vertexAccessors.size());
}

uint32_t Mesh::getVertexCount(uint32_t binding) const
{
	return m_vertexAccessors[binding].count;
}

uint32_t Mesh::getVertexOffset(uint32_t binding) const
{
	return m_vertexAccessors[binding].offset;
}

const VertexAttribute& Mesh::getVertexAttribute(uint32_t binding) const
{
	return m_vertexAccessors[binding].attribute;
}

const VertexBufferView& Mesh::getVertexBuffer(uint32_t binding) const
{
	return m_vertexAccessors[binding].bufferView;
}

void SubMesh::draw()
{
	mesh->draw(type, count, offset);
}

void SubMesh::drawIndexed()
{
	mesh->drawIndexed(type, count, offset);
}

};