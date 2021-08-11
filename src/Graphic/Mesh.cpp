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
    m_vertexStride(0),
    m_vertexCount(0),
    m_indexSize(0),
    m_indexCount(0),
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

uint32_t Mesh::getIndexCount() const
{
    return m_indexCount;
}

uint32_t Mesh::getIndexSize() const
{
    return m_indexSize;
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
	return m_vertexCount;
}

uint32_t Mesh::getVertexStride() const
{
	return m_vertexStride;
}

uint32_t Mesh::getVertexAttributeCount() const
{
	return static_cast<uint32_t>(m_vertexInfo.attributeData.size());
}

const VertexAttribute& Mesh::getVertexAttribute(uint32_t i) const
{
	return m_vertexInfo[i].attribute;
}

const SubBuffer& Mesh::getVertexBuffer(uint32_t i) const
{
	return m_vertexInfo[i].subBuffer;
}

void SubMesh::draw()
{
	mesh->draw(type, indexCount, indexOffset);
}

};