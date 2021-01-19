#pragma once

#include <stdint.h>
#include <memory>
#include <vector>

namespace aka {

enum class IndexFormat
{
	Uint8,
	Uint16,
	Uint32
};
enum class VertexFormat {
	Float,
	Float2,
	Float3,
	Float4,
	Byte4,
	Ubyte4,
	Short2,
	Ushort2,
	Short4,
	Ushort4
};

struct VertexData {;
	struct Attribute {
		uint32_t index; // Location of the attribute
		VertexFormat type;      // Type of the attribute
	};
	std::vector<Attribute> attributes;
};


class Mesh
{
public:
	using Ptr = std::shared_ptr<Mesh>;
public:
	Mesh();
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	~Mesh();

	static Mesh::Ptr create();

	void vertices(const VertexData&vertex, const void *vertices, size_t count);

	void indices(IndexFormat indexFormat, const void* indices, size_t count);

	uint32_t id() const;

	uint32_t indexSize() const;

	IndexFormat indexFormat() const;

private:
	uint32_t m_vao;
	uint32_t m_indexVbo;
	uint32_t m_vertexVbo;

	uint32_t m_indexSize;
	IndexFormat m_indexFormat;
};

};

