#pragma once

#include <stdint.h>
#include <memory>
#include <vector>

namespace aka {
// TODO merge index format & vert format as format.
enum class IndexFormat
{
	UnsignedByte,
	UnsignedShort,
	UnsignedInt
};
enum class VertexFormat {
	Float,
	Double,
	Byte,
	UnsignedByte,
	Short,
	UnsignedShort,
	Int,
	UnsignedInt
};
enum class VertexType
{
	Vec2,
	Vec3,
	Vec4,
	Mat2,
	Mat3,
	Mat4,
	Scalar,
};

uint32_t size(IndexFormat format);
uint32_t size(VertexFormat format);
uint32_t size(VertexType type);

struct VertexData {
	struct Attribute {
		uint32_t index;	   // Location of the attribute
		VertexFormat format; // Type of the attribute
		VertexType type;
	};
	std::vector<Attribute> attributes;

	uint32_t stride() const {
		uint32_t stride = 0;
		for (const VertexData::Attribute& attribute : attributes)
			stride += size(attribute.format) * size(attribute.type);
		return stride;
	}
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

