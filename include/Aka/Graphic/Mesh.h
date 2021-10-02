#pragma once

#include <stdint.h>
#include <memory>
#include <vector>

#include <Aka/Graphic/Buffer.h>

namespace aka {

enum class IndexFormat {
	UnsignedByte, // D3D11 do not support it
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
enum class VertexSemantic {
	Position,
	Normal,
	Tangent,
	TexCoord0,
	TexCoord1,
	TexCoord2,
	TexCoord3,
	Color0,
	Color1,
	Color2,
	Color3,
};

enum class PrimitiveType {
	Unknown,
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
	VertexSemantic semantic; // Semantic of the attribute
	VertexFormat format; // Format of the attribute
	VertexType type; // Type of the attribute

	uint32_t size() const { return aka::size(format) * aka::size(type); }
};

struct VertexBufferView {
	Buffer::Ptr buffer; // Buffer for vertices
	uint32_t offset; // Offset within buffer
	uint32_t size; // Size within buffer
	uint32_t stride; // Stride within buffer for interleaved data.
};

struct IndexBufferView {
	Buffer::Ptr buffer; // Buffer for indices
	uint32_t offset; // Offset within buffer
	uint32_t size; // Size within buffer
};

struct IndexAccessor {
	IndexBufferView bufferView; // Buffer view for indices
	IndexFormat format; // Format of indices
	uint32_t count; // Number of elements
};

struct VertexAccessor {
	VertexAttribute attribute; // Attribute information
	VertexBufferView bufferView; // Buffer view
	uint32_t offset; // Offset from buffer view in bytes
	uint32_t count; // Number of elements
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
	void uploadInterleaved(const VertexAttribute* attributes, size_t attributeCount, void* vertices, uint32_t vertexCount);
	// Upload interleaved indexed buffer fast
	void uploadInterleaved(const VertexAttribute* attributes, size_t attributeCount, void* vertices, uint32_t vertexCount, IndexFormat indexFormat, void* indices, uint32_t indexCount);
	// Upload the vertex and index buffer to the mesh
	virtual void upload(const VertexAccessor* vertexAccessor, size_t accessorCount, const IndexAccessor& indexAccessor) = 0;
	// Upload the vertex buffer to the mesh without indices
	virtual void upload(const VertexAccessor* vertexAccessor, size_t accessorCount) = 0;

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
	const IndexBufferView& getIndexBuffer() const;

	// Get the number of vertex attribute
	uint32_t getVertexAttributeCount() const;
	// Get the number of vertices
	uint32_t getVertexCount(uint32_t binding) const;
	// Get the number of vertices
	uint32_t getVertexOffset(uint32_t binding) const;
	// Get a single vertex attribute for given binding
	const VertexAttribute& getVertexAttribute(uint32_t binding) const;
	// Get a single vertex buffer for given binding
	const VertexBufferView& getVertexBuffer(uint32_t binding) const;

protected:
	IndexAccessor m_indexAccessor;
	std::vector<VertexAccessor> m_vertexAccessors;
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

