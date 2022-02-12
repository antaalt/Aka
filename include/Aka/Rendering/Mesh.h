#pragma once

#include <stdint.h>

#include <Aka/Graphic/Pipeline.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Rendering/Material.h>

namespace aka {

struct Mesh
{
	VertexBindingState bindings;
	Buffer* vertices[VertexBindingState::MaxAttributes]; // Vertices buffers. Maximum of 1 / binding.
	Buffer* indices; // Indices buffer

	IndexFormat format; // Index format if indexed.
	uint32_t count; // Vertices / Indices count depending on isIndexed

	bool isIndexed() const { return indices != nullptr; }

	static Mesh* create();
	static Mesh* createInterleaved(const VertexBindingState& state, const void* vertices, uint32_t vertexCount, IndexFormat indexFormat, const void* indices, uint32_t indexCount);
	static Mesh* createInterleaved(const VertexBindingState& state, const void* vertices, uint32_t vertexCount);
	static void destroy(Mesh* mesh);
};

struct SubMesh
{
	Mesh* subMeshes;
	uint32_t indexOffset;
};

};