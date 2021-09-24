#if defined(AKA_USE_D3D11)
#include "D3D11Mesh.h"

#include "D3D11Context.h"

#include <Aka/OS/Logger.h>

namespace aka {

D3D11Mesh::D3D11Mesh(D3D11Device* device) :
	Mesh(),
	m_device(device),
	m_indexFormat(DXGI_FORMAT_UNKNOWN)
{
}

D3D11Mesh::~D3D11Mesh()
{
}

void D3D11Mesh::upload(const VertexAccessor* vertexAccessor, size_t accessorCount, const IndexAccessor& indexAccessor)
{
	m_vertexAccessors = std::vector<VertexAccessor>(vertexAccessor, vertexAccessor + accessorCount);
	m_indexAccessor = indexAccessor;
	m_vertexBuffers.clear();
	m_strides.clear();
	m_offsets.clear();
	for (size_t i = 0; i < accessorCount; i++)
	{
		const VertexAccessor& a = vertexAccessor[i];
		m_vertexBuffers.push_back((ID3D11Buffer*)a.bufferView.buffer->handle().value());
		m_strides.push_back(a.bufferView.stride);
		m_offsets.push_back(a.bufferView.offset + a.offset);
	}
	switch (m_indexAccessor.format)
	{
	case IndexFormat::UnsignedByte:
		Logger::error("Unsigned byte format not supported as index buffer.");
		m_indexFormat = DXGI_FORMAT_R8_UINT;
		break;
	case IndexFormat::UnsignedShort:
		m_indexFormat = DXGI_FORMAT_R16_UINT;
		break;
	case IndexFormat::UnsignedInt:
		m_indexFormat = DXGI_FORMAT_R32_UINT;
		break;
	}
}

void D3D11Mesh::upload(const VertexAccessor* vertexAccessor, size_t accessorCount)
{
	m_vertexAccessors = std::vector<VertexAccessor>(vertexAccessor, vertexAccessor + accessorCount);
	m_indexAccessor = {};
	m_vertexBuffers.clear();
	m_strides.clear();
	m_offsets.clear();
	for (size_t i = 0; i < accessorCount; i++)
	{
		const VertexAccessor& a = vertexAccessor[i];
		m_vertexBuffers.push_back((ID3D11Buffer*)a.bufferView.buffer->handle().value());
		m_strides.push_back(a.bufferView.stride);
		m_offsets.push_back(a.bufferView.offset + a.offset);
	}
}

void D3D11Mesh::draw(PrimitiveType type, uint32_t vertexCount, uint32_t vertexOffset) const
{
	m_device->context()->IASetVertexBuffers(0, static_cast<UINT>(m_vertexBuffers.size()), m_vertexBuffers.data(), m_strides.data(), m_offsets.data());
	m_device->context()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	switch (type)
	{
	default:
	case PrimitiveType::Triangles:
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	case PrimitiveType::TriangleStrip:
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		break;
	case PrimitiveType::Points:
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		break;
	case PrimitiveType::Lines:
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		break;
	case PrimitiveType::LineStrip:
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		break;
	case PrimitiveType::LineLoop:
	case PrimitiveType::TriangleFan:
		Logger::error("Primitive type not supported");
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
		break;
	}
	m_device->context()->Draw(vertexCount, vertexOffset);
}
void D3D11Mesh::drawIndexed(PrimitiveType type, uint32_t indexCount, uint32_t indexOffset) const
{
	m_device->context()->IASetVertexBuffers(0, static_cast<UINT>(m_vertexBuffers.size()), m_vertexBuffers.data(), m_strides.data(), m_offsets.data());
	m_device->context()->IASetIndexBuffer((ID3D11Buffer*)m_indexAccessor.bufferView.buffer->handle().value(), m_indexFormat, m_indexAccessor.bufferView.offset);
	switch (type)
	{
	default:
	case PrimitiveType::Triangles:
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	case PrimitiveType::TriangleStrip:
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		break;
	case PrimitiveType::Points:
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		break;
	case PrimitiveType::Lines:
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		break;
	case PrimitiveType::LineStrip:
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		break;
	case PrimitiveType::LineLoop:
	case PrimitiveType::TriangleFan:
		Logger::error("Primitive type not supported");
		m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
		break;
	}
	m_device->context()->DrawIndexed(indexCount, indexOffset, 0);
}

};

#endif