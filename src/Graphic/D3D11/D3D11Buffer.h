#if defined(AKA_USE_D3D11)
#pragma once

#include "D3D11Context.h"

#include <Aka/Graphic/Buffer.h>

namespace aka {

class D3D11Device;

class D3D11Buffer : public Buffer
{
public:
	D3D11Buffer(D3D11Device* device, BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data);
	~D3D11Buffer();
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
	D3D11Device* m_device;
	ID3D11Buffer* m_buffer;
};

};

#endif