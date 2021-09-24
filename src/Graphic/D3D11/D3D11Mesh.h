#if defined(AKA_USE_D3D11)
#pragma once

#include "D3D11Context.h"
#include "D3D11Device.h"

#include <Aka/Graphic/Mesh.h>

namespace aka {

class D3D11Mesh : public Mesh
{
public:
	D3D11Mesh(D3D11Device* device);
	~D3D11Mesh();
public:
	void upload(const VertexAccessor* vertexAccessor, size_t accessorCount, const IndexAccessor& indexAccessor) override;
	void upload(const VertexAccessor* vertexAccessor, size_t accessorCount) override;

	void draw(PrimitiveType type, uint32_t vertexCount, uint32_t vertexOffset) const override;
	void drawIndexed(PrimitiveType type, uint32_t indexCount, uint32_t indexOffset) const override;
private:
	D3D11Device* m_device;
	DXGI_FORMAT m_indexFormat;
	std::vector<ID3D11Buffer*> m_vertexBuffers;
	std::vector<UINT> m_strides;
	std::vector<UINT> m_offsets;
};

};

#endif