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
	m_indexInfo{},
	m_vertexInfo{}
{
}

Mesh::~Mesh()
{
}

Mesh::Ptr Mesh::create()
{
	return GraphicBackend::createMesh();
}

void Mesh::uploadInterleaved(const VertexAttribute* attributes, uint32_t attributeCount, void* vertices, uint32_t vertexCount)
{
	VertexInfo info{};
	uint32_t stride = 0;
	for (uint32_t i = 0; i < attributeCount; i++)
		stride += attributes[i].size();
	SubBuffer buffer{};
	buffer.buffer = Buffer::create(BufferType::VertexBuffer, vertexCount * stride, BufferUsage::Static, BufferAccess::ReadOnly, vertices);
	buffer.size = vertexCount * stride;
	buffer.offset = 0;
	uint32_t offset = 0;
	for (uint32_t i = 0; i < attributeCount; i++)
	{
		info.attributeData.emplace_back(VertexAttributeData{ attributes[i], buffer, stride, offset });
		offset += attributes[i].size();
	}
	upload(info);
}

void Mesh::uploadInterleaved(const VertexAttribute* attributes, uint32_t attributeCount, void* vertices, uint32_t vertexCount, IndexFormat indexFormat, void* indices, uint32_t indexCount)
{
	VertexInfo vertexInfo{};
	uint32_t stride = 0;
	for (uint32_t i = 0; i < attributeCount; i++)
		stride += attributes[i].size();
	SubBuffer buffer{};
	buffer.buffer = Buffer::create(BufferType::VertexBuffer, vertexCount * stride, BufferUsage::Static, BufferAccess::ReadOnly, vertices);
	buffer.size = vertexCount * stride;
	buffer.offset = 0;
	uint32_t offset = 0;
	for (uint32_t i = 0; i < attributeCount; i++)
	{
		vertexInfo.attributeData.emplace_back(VertexAttributeData{ attributes[i], buffer, stride, offset });
		offset += attributes[i].size();
	}
	IndexInfo indexInfo{};
	indexInfo.format = IndexFormat::UnsignedInt;
	indexInfo.subBuffer.buffer = Buffer::create(BufferType::IndexBuffer, indexCount * 4, BufferUsage::Static, BufferAccess::ReadOnly, indices);
	indexInfo.subBuffer.offset = 0;
	indexInfo.subBuffer.size = indexCount * 4;
	upload(vertexInfo, indexInfo);
}

bool Mesh::isIndexed() const
{
	return m_indexInfo.subBuffer.buffer != nullptr;
}

uint32_t Mesh::getIndexCount() const
{
	return m_indexInfo.subBuffer.size / size(m_indexInfo.format);
}

IndexFormat Mesh::getIndexFormat() const
{
    return m_indexInfo.format;
}

const SubBuffer& Mesh::getIndexBuffer() const
{
	return m_indexInfo.subBuffer;
}

uint32_t Mesh::getVertexCount() const
{
	if (m_vertexInfo.attributeData.size() == 0) return 0;
	return m_vertexInfo.attributeData[0].subBuffer.size / m_vertexInfo.attributeData[0].attribute.size();
}

uint32_t Mesh::getVertexAttributeCount() const
{
	return static_cast<uint32_t>(m_vertexInfo.attributeData.size());
}

const VertexAttribute& Mesh::getVertexAttribute(uint32_t binding) const
{
	return m_vertexInfo[binding].attribute;
}

const SubBuffer& Mesh::getVertexBuffer(uint32_t binding) const
{
	return m_vertexInfo[binding].subBuffer;
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