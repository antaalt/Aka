#pragma once

#include <Aka/Resource/Storage.h>
#include <Aka/Resource/ResourceAllocator.h>

#include <Aka/Rendering/Mesh.h>

namespace aka {

using MeshAllocator = ResourceAllocator<Mesh>;

struct MeshStorage : IStorage<Mesh>
{
	static constexpr uint16_t major = 0;
	static constexpr uint16_t minor = 1;

	// Vertices
	struct VertexBinding {
		gfx::VertexAttribute attribute;
		String vertexBufferName;
		uint32_t vertexCount;
		uint32_t vertexOffset;
		uint32_t vertexBufferOffset;
		uint32_t vertexBufferSize;
		uint32_t vertexBufferStride;
	};
	std::vector<VertexBinding> vertices;

	// Indices
	gfx::IndexFormat indexFormat;
	uint32_t indexCount;
	uint32_t indexBufferOffset;
	String indexBufferName;


	bool load(const Path& path) override;
	bool save(const Path& path) const override;

	Mesh* allocate() const override;
	void deallocate(Mesh* mesh) const override;
	void serialize(const Mesh& mesh) override;

	size_t size(const Mesh& mesh) override;
};

};