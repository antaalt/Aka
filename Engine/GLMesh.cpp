#include "Mesh.h"

#include "GLBackend.h"

namespace aka {

Mesh::Mesh() :
    m_vao(0),
    m_indexVbo(0),
    m_vertexVbo(0)
{
    glGenVertexArrays(1, &m_vao);
}

Mesh::~Mesh()
{
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
    if (m_indexVbo)
        glDeleteBuffers(1, &m_indexVbo);
    if (m_vertexVbo)
        glDeleteBuffers(1, &m_vertexVbo);
}

Mesh::Ptr Mesh::create()
{
	return std::make_shared<Mesh>();
}

void Mesh::vertices(const Vertex& vertex, const void* vertices, size_t count)
{
    glBindVertexArray(m_vao);
    if (m_vertexVbo == 0)
        glGenBuffers(1, &m_vertexVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVbo);
    size_t ptr = 0;
    for (const Vertex::Attribute &attribute : vertex.attributes)
    {
        GLsizei componentSize = 0;
        GLint components = 1;
        GLboolean normalized = GL_FALSE;
        GLenum type = GL_FLOAT;
        switch (attribute.type)
        {
        case Vertex::Type::Float:
            type = GL_FLOAT;
            componentSize = 4;
            components = 1;
            break;
        case Vertex::Type::Float2:
            type = GL_FLOAT;
            componentSize = 4;
            components = 2;
            break;
        case Vertex::Type::Float3:
            type = GL_FLOAT;
            componentSize = 4;
            components = 3;
            break;
        case Vertex::Type::Float4:
            type = GL_FLOAT;
            componentSize = 4;
            components = 4;
            break;
        case Vertex::Type::Byte4:
            type = GL_BYTE;
            componentSize = 1;
            components = 4;
            break;
        case Vertex::Type::Ubyte4:
            type = GL_UNSIGNED_BYTE;
            componentSize = 1;
            components = 4;
            break;
        case Vertex::Type::Short2:
            type = GL_SHORT;
            componentSize = 2;
            components = 2;
            break;
        case Vertex::Type::Ushort2:
            type = GL_UNSIGNED_SHORT;
            componentSize = 2;
            components = 2;
            break;
        case Vertex::Type::Short4:
            type = GL_SHORT;
            componentSize = 2;
            components = 4;
            break;
        case Vertex::Type::Ushort4:
            type = GL_UNSIGNED_SHORT;
            componentSize = 2;
            components = 4;
            break;
        }
        glEnableVertexAttribArray(attribute.index);
        // TODO check componentSize * components, use fixed stride ?
        glVertexAttribPointer(attribute.index, components, type, normalized, componentSize * components, (void*)ptr);
        ptr += components * componentSize;
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::indices(IndexType indexType, const void* indices, size_t count)
{
    glBindVertexArray(m_vao);
    if (m_indexVbo == 0)
        glGenBuffers(1, &m_indexVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVbo);
    size_t indexSize = 0;
    switch (indexType)
    {
    case IndexType::Uint8:
        indexSize = 1;
        break;
    case IndexType::Uint16:
        indexSize = 2;
        break;
    case IndexType::Uint32:
        indexSize = 4;
        break;
    }
    // GL_DYNAMIC_DRAW so we can change buffer data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * count, indices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

};