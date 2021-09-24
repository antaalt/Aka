#if defined(AKA_USE_OPENGL)
#include "GLMesh.h"

#include "GLContext.h"

namespace aka {

GLMesh::GLMesh() :
	Mesh(),
	m_vao(0)
{
	glGenVertexArrays(1, &m_vao);
}
GLMesh::~GLMesh()
{
	if (m_vao)
		glDeleteVertexArrays(1, &m_vao);
}
void GLMesh::upload(const VertexAccessor* vertexAccessor, size_t accessorCount, const IndexAccessor& indexAccessor)
{
	// --- Vertices
	glBindVertexArray(m_vao);
	// Setup correct channels
	for (size_t i = m_vertexAccessors.size(); i < accessorCount; i++)
		glEnableVertexAttribArray((GLuint)i);
	for (size_t i = accessorCount; i < m_vertexAccessors.size(); i++)
		glDisableVertexAttribArray((GLuint)i);
	for (size_t i = 0; i < accessorCount; i++)
	{
		const VertexAccessor& a = vertexAccessor[i];
		//GLint componentSize = size(a.attribute.format);
		GLint componentCount = size(a.attribute.type);
		GLenum componentType = glFormat(a.attribute.format);
		GLboolean normalized = GL_FALSE;
		GLsizei stride = a.bufferView.stride;
		size_t offset = a.bufferView.offset + a.offset;
		glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(a.bufferView.buffer->handle().value()));
		glVertexAttribPointer((GLuint)i, componentCount, componentType, normalized, stride, (void*)(uintptr_t)offset);
	}
	m_vertexAccessors.clear();
	m_vertexAccessors.insert(m_vertexAccessors.end(), vertexAccessor, vertexAccessor + accessorCount);

	// --- Indices
	if (indexAccessor.bufferView.buffer != nullptr)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(indexAccessor.bufferView.buffer->handle().value()));
		m_indexAccessor = indexAccessor;
	}
	else
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		m_indexAccessor = {};
	}
	glBindVertexArray(0);
}

void GLMesh::upload(const VertexAccessor* vertexAccessor, size_t accessorCount)
{
	upload(vertexAccessor, accessorCount, IndexAccessor{ IndexBufferView{ nullptr, 0, 0 }, IndexFormat::UnsignedByte });
}

void GLMesh::draw(PrimitiveType type, uint32_t vertexCount, uint32_t vertexOffset) const
{
	glBindVertexArray(m_vao);
	glDrawArrays(glPrimitive(type), vertexOffset, vertexCount);
	glBindVertexArray(0);
}
void GLMesh::drawIndexed(PrimitiveType type, uint32_t indexCount, uint32_t indexOffset) const
{
	AKA_ASSERT(m_indexAccessor.bufferView.buffer != nullptr, "Need indices to call drawIndexed");
	glBindVertexArray(m_vao);
	glDrawElements(
		glPrimitive(type),
		static_cast<GLsizei>(indexCount),
		glFormat(m_indexAccessor.format),
		(void*)((uintptr_t)size(m_indexAccessor.format) * (uintptr_t)indexOffset)
	);
	glBindVertexArray(0);
}

};

#endif