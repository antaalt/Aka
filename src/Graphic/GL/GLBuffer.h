#if defined(AKA_USE_OPENGL)
#pragma once

#include "GLContext.h"

#include <Aka/Graphic/Buffer.h>

namespace aka {

class GLBuffer : public Buffer
{
public:
	GLBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data);
	~GLBuffer();
	void reallocate(size_t size, const void* data) override;
	void upload(const void* data, size_t size, size_t offset = 0) override;
	void upload(const void* data) override;
	void download(void* data, size_t size, size_t offset = 0) override;
	void download(void* data) override;
	void* map(BufferMap access) override;
	void unmap() override;
	BufferHandle handle() const override;
	void copy(const Buffer::Ptr& buffer, size_t offsetSRC, size_t offsetDST, size_t size) override;
private:
	GLenum m_glType;
	GLuint m_bufferID;
};

};

#endif