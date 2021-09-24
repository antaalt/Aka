#if defined(AKA_USE_OPENGL)
#pragma once

#include "GLContext.h"

#include <Aka/Graphic/Mesh.h>

namespace aka {

class GLMesh : public Mesh
{
public:
	GLMesh();
	~GLMesh();
public:
	void upload(const VertexAccessor* vertexAccessor, size_t accessorCount, const IndexAccessor& indexAccessor) override;
	void upload(const VertexAccessor* vertexAccessor, size_t accessorCount) override;

	void draw(PrimitiveType type, uint32_t vertexCount, uint32_t vertexOffset) const override;
	void drawIndexed(PrimitiveType type, uint32_t indexCount, uint32_t indexOffset) const override;
private:
	GLuint m_vao;
};

};

#endif