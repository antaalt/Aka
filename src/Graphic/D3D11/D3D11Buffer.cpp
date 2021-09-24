#if defined(AKA_USE_D3D11)
#include "D3D11Buffer.h"

#include "D3D11Context.h"
#include "D3D11Device.h"

#include <Aka/OS/Logger.h>

namespace aka {

D3D11Buffer::D3D11Buffer(D3D11Device* device, BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data) :
	Buffer(type, size, usage, access),
	m_device(device),
	m_buffer(nullptr)
{
	reallocate(size, data);
}
D3D11Buffer::~D3D11Buffer()
{
	if (m_buffer)
		m_buffer->Release();
}

void D3D11Buffer::reallocate(size_t size, const void* data)
{
	if (m_buffer)
		m_buffer->Release();
	D3D11_BUFFER_DESC bufferDesc{};
	switch (m_usage)
	{
	case BufferUsage::Default:
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		break;
	case BufferUsage::Immutable:
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		break;
	case BufferUsage::Dynamic:
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		break;
	case BufferUsage::Staging:
		bufferDesc.Usage = D3D11_USAGE_STAGING;
		break;
	}
	switch (m_type)
	{
	case BufferType::Vertex:
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		break;
	case BufferType::Index:
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		break;
	case BufferType::Uniform:
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		break;
	case BufferType::ShaderStorage:
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		break;
	default:
		Logger::warn("Unkwnown buffer type.");
		bufferDesc.BindFlags = 0;
		break;
	}
	if ((m_usage == BufferUsage::Staging) || (m_usage == BufferUsage::Dynamic && (m_access == BufferCPUAccess::Write || m_access == BufferCPUAccess::None)))
	{
		switch (m_access)
		{
		case BufferCPUAccess::Read:
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			break;
		case BufferCPUAccess::Write:
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			break;
		default:
		case BufferCPUAccess::ReadWrite:
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			break;
		}
	}
	else
	{
		if (m_access != BufferCPUAccess::None)
			Logger::error("Cannot set given BufferCPUAccess for given BufferUsage.");
		bufferDesc.CPUAccessFlags = 0;
	}
	bufferDesc.ByteWidth = static_cast<UINT>(size);
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subData{};
	subData.pSysMem = data;
	subData.SysMemPitch = 0;
	subData.SysMemSlicePitch = 0;

	D3D11_SUBRESOURCE_DATA* pSubData = nullptr;
	if (data != nullptr)
		pSubData = &subData;
	D3D_CHECK_RESULT(m_device->device()->CreateBuffer(&bufferDesc, pSubData, &m_buffer));
}

void D3D11Buffer::upload(const void* data, size_t size, size_t offset)
{
	AKA_ASSERT(m_usage != BufferUsage::Dynamic, "Do not work with dynamic buffer. Use map instead.");
	D3D11_BOX box{};
	box.left = (UINT)offset;
	box.right = (UINT)(offset + size);
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;
	m_device->context()->UpdateSubresource(
		m_buffer,
		0,
		&box,
		data,
		0,
		0
	);
}

void D3D11Buffer::upload(const void* data)
{
	m_device->context()->UpdateSubresource(
		m_buffer,
		0,
		nullptr,
		data,
		0,
		0
	);
}

void D3D11Buffer::download(void* data, size_t size, size_t offset)
{
	// TODO create staging buffer to download non dynamic buffer
	void* d = map(BufferMap::Read);
	memcpy(data, static_cast<char*>(d) + offset, size);
	unmap();
}

void D3D11Buffer::download(void* data)
{
	download(data, m_size, 0);
}

void* D3D11Buffer::map(BufferMap access)
{
	AKA_ASSERT(m_usage == BufferUsage::Dynamic, "Only works for dynamic buffer. Use upload instead.");
	D3D11_MAP map;
	switch (access)
	{
	default:
	case BufferMap::Read:
		map = D3D11_MAP_READ;
		break;
	case BufferMap::Write:
		map = D3D11_MAP_WRITE;
		break;
	case BufferMap::ReadWrite:
		map = D3D11_MAP_READ_WRITE;
		break;
	case BufferMap::WriteDiscard:
		map = D3D11_MAP_WRITE_DISCARD;
		break;
	case BufferMap::WriteNoOverwrite:
		map = D3D11_MAP_WRITE_NO_OVERWRITE;
		break;
	}
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	D3D_CHECK_RESULT(m_device->context()->Map(m_buffer, 0, map, 0, &mappedResource));
	return mappedResource.pData;
}

void D3D11Buffer::unmap()
{
	m_device->context()->Unmap(m_buffer, 0);
}

BufferHandle D3D11Buffer::handle() const
{
	return BufferHandle((uintptr_t)m_buffer);
}

void D3D11Buffer::copy(const Buffer::Ptr& buffer, size_t offsetSRC, size_t offsetDST, size_t size)
{
	D3D11_BOX box{};
	box.left = (UINT)offsetSRC;
	box.right = (UINT)offsetSRC + (UINT)size;
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;

	D3D11_BOX* pBox = nullptr;
	if (offsetSRC != offsetDST)
		pBox = &box;
	m_device->context()->CopySubresourceRegion(reinterpret_cast<D3D11Buffer*>(buffer.get())->m_buffer, 0, (UINT)offsetDST, 0, 0, m_buffer, 0, pBox);
}

};

#endif