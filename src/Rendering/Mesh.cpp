#include <Aka/Rendering/Mesh.h>

namespace aka {

Mesh* Mesh::create()
{
	return new Mesh; // TODO pool
}

Mesh* Mesh::createInterleaved(const gfx::VertexBindingState& state, const void* vertices, uint32_t vertexCount, gfx::IndexFormat indexFormat, const void* indices, uint32_t indexCount)
{
	Mesh* mesh = new Mesh; // TODO pool
	// TODO pass bindings as arguments
	mesh->bindings = state;
	//for (uint32_t i = 0; i < bindings.)
	mesh->vertices[0] = gfx::Buffer::createVertexBuffer(state.stride() * vertexCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, vertices);
	mesh->indices = gfx::Buffer::createIndexBuffer(gfx::VertexBindingState::size(indexFormat) * indexCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, indices);
	mesh->count = indexCount;
	mesh->format = indexFormat;
	return mesh;
}

Mesh* Mesh::createInterleaved(const gfx::VertexBindingState& state, const void* vertices, uint32_t vertexCount)
{
	Mesh* mesh = new Mesh; // TODO pool
	// TODO pass bindings as arguments
	mesh->bindings = state;
	//for (uint32_t i = 0; i < bindings.)
	mesh->vertices[0] = gfx::Buffer::createVertexBuffer(state.stride() * vertexCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, vertices);
	mesh->indices = nullptr;
	mesh->count = vertexCount;
	mesh->format = gfx::IndexFormat::Unknown;
	return mesh;
}

void Mesh::destroy(Mesh* mesh)
{
	delete mesh;
}

};