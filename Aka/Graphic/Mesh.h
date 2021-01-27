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

struct VertexData {
	struct Attribute {
		uint32_t index;	   // Location of the attribute
		VertexFormat type; // Type of the attribute
	};
	std::vector<Attribute> attributes;
};

class Mesh
{
public:
	using Ptr = std::shared_ptr<Mesh>;
protected:
	Mesh();
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	virtual ~Mesh();
public:
	static Mesh::Ptr create();

	virtual void vertices(const VertexData&vertex, const void *vertices, size_t count) = 0;

	virtual void indices(IndexFormat indexFormat, const void* indices, size_t count) = 0;

	void draw() const { draw(m_indexCount, 0); }
	virtual void draw(uint32_t indexCount, uint32_t indexOffset) const = 0;

	uint32_t getIndexCount() const;
	uint32_t getIndexSize() const;
	IndexFormat getIndexFormat() const;
	const VertexData &getVertexData() const;

protected:
	uint32_t m_vertexStride;
	uint32_t m_vertexCount;
	uint32_t m_indexSize;
	uint32_t m_indexCount;
	IndexFormat m_indexFormat;
	VertexData m_vertexData;
};

};

