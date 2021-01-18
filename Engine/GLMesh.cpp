#include "Mesh.h"

#include "GLBackend.h"

namespace aka {

Mesh::Mesh() :
    m_vao(0),
    m_indexVbo(0),
    m_vertexVbo(0),
    m_indexSize(0),
    m_indexFormat(IndexFormat::Uint32)
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

void Mesh::vertices(const VertexData& vertex, const void* vertices, size_t count)
{
    glBindVertexArray(m_vao);
    if (m_vertexVbo == 0)
        glGenBuffers(1, &m_vertexVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVbo);
    uint32_t stride = 0;
    for (const VertexData::Attribute& attribute : vertex.attributes)
    {
        switch (attribute.type)
        {
        case VertexFormat::Float: stride += 4; break;
        case VertexFormat::Float2: stride += 8; break;
        case VertexFormat::Float3: stride += 12; break;
        case VertexFormat::Float4: stride += 16; break;
        case VertexFormat::Byte4: stride += 4; break;
        case VertexFormat::Ubyte4: stride += 4; break;
        case VertexFormat::Short2: stride += 4; break;
        case VertexFormat::Ushort2: stride += 4; break;
        case VertexFormat::Short4: stride += 8; break;
        case VertexFormat::Ushort4: stride += 8; break;
        }
    }
    size_t ptr = 0;
    for (const VertexData::Attribute &attribute : vertex.attributes)
    {
        GLint componentSize = 0;
        GLint components = 1;
        GLboolean normalized = GL_FALSE;
        GLenum type = GL_FLOAT;
        switch (attribute.type)
        {
        case VertexFormat::Float:
            type = GL_FLOAT;
            componentSize = 4;
            components = 1;
            break;
        case VertexFormat::Float2:
            type = GL_FLOAT;
            componentSize = 4;
            components = 2;
            break;
        case VertexFormat::Float3:
            type = GL_FLOAT;
            componentSize = 4;
            components = 3;
            break;
        case VertexFormat::Float4:
            type = GL_FLOAT;
            componentSize = 4;
            components = 4;
            break;
        case VertexFormat::Byte4:
            type = GL_BYTE;
            componentSize = 1;
            components = 4;
            break;
        case VertexFormat::Ubyte4:
            type = GL_UNSIGNED_BYTE;
            componentSize = 1;
            components = 4;
            break;
        case VertexFormat::Short2:
            type = GL_SHORT;
            componentSize = 2;
            components = 2;
            break;
        case VertexFormat::Ushort2:
            type = GL_UNSIGNED_SHORT;
            componentSize = 2;
            components = 2;
            break;
        case VertexFormat::Short4:
            type = GL_SHORT;
            componentSize = 2;
            components = 4;
            break;
        case VertexFormat::Ushort4:
            type = GL_UNSIGNED_SHORT;
            componentSize = 2;
            components = 4;
            break;
        }
        glEnableVertexAttribArray(attribute.index);
        glVertexAttribPointer(attribute.index, components, type, normalized, stride, (void*)ptr);
        ptr += components * componentSize;
    }

    glBufferData(GL_ARRAY_BUFFER, stride * count, vertices, GL_DYNAMIC_DRAW);
    // Do not unbind buffers as they will be unbind from VAO
    glBindVertexArray(0);
}

void Mesh::indices(IndexFormat format, const void* indices, size_t count)
{
    glBindVertexArray(m_vao);
    if (m_indexVbo == 0)
        glGenBuffers(1, &m_indexVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVbo);
    switch (format)
    {
    case IndexFormat::Uint8:
        m_indexFormat = IndexFormat::Uint8;
        m_indexSize = 1;
        break;
    case IndexFormat::Uint16:
        m_indexFormat = IndexFormat::Uint16;
        m_indexSize = 2;
        break;
    case IndexFormat::Uint32:
        m_indexFormat = IndexFormat::Uint32;
        m_indexSize = 4;
        break;
    }
    // GL_DYNAMIC_DRAW so we can change buffer data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexSize * count, indices, GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
}

uint32_t Mesh::id() const
{
    return m_vao;
}

uint32_t Mesh::indexSize() const
{
    return m_indexSize;
}

IndexFormat Mesh::indexFormat() const
{
    return m_indexFormat;
}

};