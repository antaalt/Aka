#pragma once

#include <Aka/Resource/Storage.h>
#include <Aka/Resource/ResourceAllocator.h>

#include <Aka/Graphic/Mesh.h>

namespace aka {

using MeshAllocator = ResourceAllocator<Mesh>;

struct MeshStorage : IStorage<Mesh>
{
	static constexpr uint16_t major = 0;
	static constexpr uint16_t minor = 1;

	// Vertices
	struct Vertex {
		VertexAttribute attribute;
		String vertexBufferName;
		uint32_t vertexCount;
		uint32_t vertexOffset;
		uint32_t vertexBufferOffset;
		uint32_t vertexBufferSize;
		uint32_t vertexBufferStride;
	};
	std::vector<Vertex> vertices;

	// Indices
	IndexFormat indexFormat;
	uint32_t indexCount;
	uint32_t indexBufferOffset;
	String indexBufferName;


	bool load(const Path& path) override;
	bool save(const Path& path) const override;

	std::shared_ptr<Mesh> to() const override;
	void from(const std::shared_ptr<Mesh>& mesh) override;

	size_t size(const std::shared_ptr<Mesh>& mesh) override;
};

};