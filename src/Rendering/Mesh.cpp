#include <Aka/Rendering/Mesh.h>

namespace aka {

Mesh* Mesh::create()
{
	return new Mesh; // TODO pool
}

Mesh* Mesh::createInterleaved(const VertexBindingState& state, const void* vertices, uint32_t vertexCount, IndexFormat indexFormat, const void* indices, uint32_t indexCount)
{
	Mesh* mesh = new Mesh; // TODO pool
	// TODO pass bindings as arguments
	mesh->bindings = state;
	//for (uint32_t i = 0; i < bindings.)
	mesh->vertices[0] = Buffer::createVertexBuffer(state.stride() * vertexCount, BufferUsage::Default, BufferCPUAccess::None, vertices);
	mesh->indices = Buffer::createIndexBuffer(VertexBindingState::size(indexFormat) * indexCount, BufferUsage::Default, BufferCPUAccess::None, indices);
	mesh->count = indexCount;
	mesh->format = indexFormat;
	return mesh;
}

Mesh* Mesh::createInterleaved(const VertexBindingState& state, const void* vertices, uint32_t vertexCount)
{
	Mesh* mesh = new Mesh; // TODO pool
	// TODO pass bindings as arguments
	mesh->bindings = state;
	//for (uint32_t i = 0; i < bindings.)
	mesh->vertices[0] = Buffer::createVertexBuffer(state.stride() * vertexCount, BufferUsage::Default, BufferCPUAccess::None, vertices);
	mesh->indices = nullptr;
	mesh->count = vertexCount;
	mesh->format = IndexFormat::Unknown;
	return mesh;
}

void Mesh::destroy(Mesh* mesh)
{
	delete mesh;
}

};