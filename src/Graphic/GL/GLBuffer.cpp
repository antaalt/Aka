#if defined(AKA_USE_OPENGL)
#include "GLBuffer.h"

#include "GLContext.h"

namespace aka {

GLBuffer::GLBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data) :
	Buffer(type, size, usage, access),
	m_glType(glType(type)),
	m_bufferID(0)
{
	glGenBuffers(1, &m_bufferID);

	glBindBuffer(m_glType, m_bufferID);
	glBufferData(m_glType, size, data, glAccess(usage, access));
}
GLBuffer::~GLBuffer()
{
	if (m_bufferID)
		glDeleteBuffers(1, &m_bufferID);
}

void GLBuffer::reallocate(size_t size, const void* data)
{
	glBindBuffer(m_glType, m_bufferID);
	glBufferData(m_glType, size, data, glAccess(m_usage, m_access));
}

void GLBuffer::upload(const void* data, size_t size, size_t offset)
{
	AKA_ASSERT(m_usage != BufferUsage::Immutable, "Cannot upload to static buffer.");
	glBindBuffer(m_glType, m_bufferID);
	glBufferSubData(m_glType, offset, size, data);
}

void GLBuffer::upload(const void* data)
{
	AKA_ASSERT(m_usage != BufferUsage::Immutable, "Cannot upload to static buffer.");
	glBindBuffer(m_glType, m_bufferID);
	glBufferSubData(m_glType, 0, m_size, data);
}

void GLBuffer::download(void* data, size_t size, size_t offset)
{
	glBindBuffer(m_glType, m_bufferID);
	glGetBufferSubData(m_glType, offset, size, data);
}

void GLBuffer::download(void* data)
{
	glBindBuffer(m_glType, m_bufferID);
	glGetBufferSubData(m_glType, 0, m_size, data);
}

void* GLBuffer::map(BufferMap access)
{
	glBindBuffer(m_glType, m_bufferID);
	GLenum glAccess = GL_READ_ONLY;
	switch (access)
	{
	case BufferMap::Read:
		AKA_ASSERT(m_access == BufferCPUAccess::Read || m_access == BufferCPUAccess::ReadWrite, "");
		glAccess = GL_READ_ONLY;
		break;
	case BufferMap::WriteDiscard:
	case BufferMap::WriteNoOverwrite:
	case BufferMap::Write:
		AKA_ASSERT(m_access == BufferCPUAccess::Write || m_access == BufferCPUAccess::ReadWrite, "");
		glAccess = GL_WRITE_ONLY;
		break;
	case BufferMap::ReadWrite:
		AKA_ASSERT(m_access == BufferCPUAccess::ReadWrite, "");
		glAccess = GL_READ_WRITE;
		break;
	default:
		return nullptr;
	}
	void* data = glMapBuffer(m_glType, glAccess);
	return data;
}

void GLBuffer::unmap()
{
	glBindBuffer(m_glType, m_bufferID);
	glUnmapBuffer(m_glType);
}

BufferHandle GLBuffer::handle() const
{
	return BufferHandle(m_bufferID);
}

void GLBuffer::copy(const Buffer::Ptr& buffer, size_t offsetSRC, size_t offsetDST, size_t size)
{
	glBindBuffer(GL_COPY_READ_BUFFER, m_bufferID);
	glBindBuffer(GL_COPY_WRITE_BUFFER, reinterpret_cast<GLBuffer*>(buffer.get())->m_bufferID);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, offsetSRC, offsetDST, size);
}

};

#endif