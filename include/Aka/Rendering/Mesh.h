#pragma once

#include <stdint.h>

#include <Aka/Graphic/Pipeline.h>
#include <Aka/Graphic/Buffer.h>

namespace aka {

struct Mesh
{
	gfx::VertexBindingState bindings;
	gfx::Buffer* vertices[gfx::VertexBindingState::MaxAttributes]; // Vertices buffers. Maximum of 1 / binding.
	gfx::Buffer* indices; // Indices buffer

	gfx::IndexFormat format; // Index format if indexed.
	uint32_t count; // Vertices / Indices count depending on isIndexed

	bool isIndexed() const { return indices != nullptr; }

	static Mesh* create();
	static Mesh* createInterleaved(const gfx::VertexBindingState& state, const void* vertices, uint32_t vertexCount, gfx::IndexFormat indexFormat, const void* indices, uint32_t indexCount);
	static Mesh* createInterleaved(const gfx::VertexBindingState& state, const void* vertices, uint32_t vertexCount);
	static void destroy(Mesh* mesh);
};

struct SubMesh
{
	Mesh* subMeshes;
	uint32_t indexOffset;
};

};