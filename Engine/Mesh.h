#pragma once

#include <stdint.h>
#include <memory>
#include <vector>

namespace aka {

enum class IndexType
{
	Uint8,
	Uint16,
	Uint32
};

struct Vertex {
	enum class Type {
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
	struct Attribute {
		uint32_t index; // Location of the attribute
		Type type;      // Type of the attribute
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

	void vertices(const Vertex &vertex, const void *vertices, size_t count);

	void indices(IndexType indexType, const void* indices, size_t count);

private:
	uint32_t m_vao;
	uint32_t m_indexVbo;
	uint32_t m_vertexVbo;
};

};

