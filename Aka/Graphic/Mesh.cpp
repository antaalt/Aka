#include "Mesh.h"
#include "GraphicBackend.h"

namespace aka {

Mesh::Mesh() :
    m_vertexStride(0),
    m_vertexCount(0),
    m_indexSize(0),
    m_indexCount(0),
    m_indexFormat(IndexFormat::Uint32)
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
    return m_indexFormat;
}

const VertexData& Mesh::getVertexData() const
{
    return m_vertexData;
}

};