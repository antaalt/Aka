#pragma once

#include <stdint.h>
#include <memory>
#include <vector>

#include <Aka/Graphic/Buffer.h>

namespace aka {

enum class IndexFormat {
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

enum class VertexType {
	Vec2,
	Vec3,
	Vec4,
	Mat2,
	Mat3,
	Mat4,
	Scalar,
};

enum class PrimitiveType {
	Points,
	LineStrip,
	LineLoop,
	Lines,
	TriangleStrip,
	TriangleFan,
	Triangles,
};

uint32_t size(IndexFormat format);
uint32_t size(VertexFormat format);
uint32_t size(VertexType type);

struct VertexAttribute {
	VertexFormat format; // Format of the attribute
	VertexType type; // Type of the attribute

	uint32_t size() const { return aka::size(format) * aka::size(type); }
};

struct VertexAttributeData {
	VertexAttribute attribute; // Attribute information
	SubBuffer subBuffer; // Buffer used to store the attribute
	uint32_t stride; // Stride in the buffer
	uint32_t offset; // Offset from base vertex
};

struct VertexInfo {
	VertexAttributeData& operator[](size_t index) { return attributeData[index]; }
	const VertexAttributeData& operator[](size_t index) const { return attributeData[index]; }

	std::vector<VertexAttributeData> attributeData; // index in vector is location in shader
};

struct IndexInfo {
	IndexFormat format;
	SubBuffer subBuffer;
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

	// Upload interleaved buffer fast
	void uploadInterleaved(const VertexAttribute* attributes, uint32_t attributeCount, void* vertices, uint32_t vertexCount);
	// Upload interleaved indexed buffer fast
	void uploadInterleaved(const VertexAttribute* attributes, uint32_t attributeCount, void* vertices, uint32_t vertexCount, IndexFormat indexFormat, void* indices, uint32_t indexCount);
	// Upload the vertex and index buffer to the mesh
	virtual void upload(const VertexInfo& vertexBuffer, const IndexInfo& indexBuffer) = 0;
	// Upload the vertex buffer to the mesh without indices
	virtual void upload(const VertexInfo& vertexBuffer) = 0;

	// Draw a mesh with only vertices
	virtual void draw(PrimitiveType type, uint32_t vertexCount, uint32_t vertexOffset) const = 0;
	// Draw a mesh with indices
	virtual void drawIndexed(PrimitiveType type, uint32_t indexCount, uint32_t indexOffset) const = 0;

	// Check if the mesh is using indices
	bool isIndexed() const;
	// Get the number of indices
	uint32_t getIndexCount() const;
	// Get the format used by the indices
	IndexFormat getIndexFormat() const;
	// Get the index buffer
	const SubBuffer& getIndexBuffer() const;

	// Get the number of vertices
	uint32_t getVertexCount() const;
	// Get the number of vertex attribute
	uint32_t getVertexAttributeCount() const;
	// Get a single vertex attribute for given binding
	const VertexAttribute& getVertexAttribute(uint32_t binding) const;
	// Get a single vertex buffer for given binding
	const SubBuffer& getVertexBuffer(uint32_t binding) const;

protected:
	IndexInfo m_indexInfo;
	VertexInfo m_vertexInfo;
};

struct SubMesh {
	Mesh::Ptr mesh;
	PrimitiveType type;
	uint32_t count; // number of element (indices or vertices)
	uint32_t offset; // offset to the first element (indices or vertices)

	void draw();
	void drawIndexed();
};

};

