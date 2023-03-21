#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Resource.h>
#include <Aka/Core/Container/Blob.h>

#include <type_traits>

namespace aka {

enum class StaticMeshBuildFlag : uint8_t
{
	None = 0,
	//IndexedDraw = 1 << 0,
	//IndirectDraw = 1 << 1,
};

AKA_IMPLEMENT_BITMASK_OPERATOR(StaticMeshBuildFlag);

struct StaticMeshRenderData : RenderData
{
	struct VertexAttribute
	{
		uint32_t count; // Vertex count in binding
		uint32_t offset; // Vertex offset in binding
		uint32_t binding; // Vertex binding
	};

	// Indices
	gfx::BufferHandle indexBuffer;
	gfx::IndexFormat indexFormat;
	uint32_t indexCount; // Indices count in buffer
	uint32_t indexOffset; // Indices offset in buffer

	// Vertices
	gfx::VertexAttributeState attributeState;
	gfx::BufferHandle vertexBuffers[gfx::VertexMaxAttributeCount];
	VertexAttribute vertexAttributes[gfx::VertexMaxAttributeCount];
	uint32_t vertexBufferCount; // Buffer count
	uint32_t vertexCount; // Vertex count to draw
	uint32_t vertexOffset; // Vertex offset to draw
};

struct StaticMeshBuildData : BuildData
{
	struct VertexAttribute
	{
		uint32_t vertexBufferIndex; // Buffer to use
		uint32_t vertexBufferOffset; // Offset in buffer
		uint32_t vertexBufferSize; // Size in buffer
		gfx::VertexAttribute attribute;
		uint32_t getVertexCount(const Blob* buffers) const { return (uint32_t)buffers[vertexBufferIndex].size() / gfx::VertexAttributeState::size(attribute.format) * gfx::VertexAttributeState::size(attribute.type); }
	};

	// Indices
	Blob indicesBuffer;
	gfx::IndexFormat indexFormat;
	uint32_t getIndexCount() const { return (uint32_t)indicesBuffer.size() / gfx::VertexAttributeState::size(indexFormat); }

	// Vertices
	Blob vertexBuffers[gfx::VertexMaxAttributeCount]; // Every buffers for vertices
	VertexAttribute vertexAttributes[gfx::VertexMaxAttributeCount]; // Every buffers for vertices
	uint32_t vertexAttributeCount;
	uint32_t vertexBufferCount;
	uint32_t vertexCount;
	uint32_t vertexOffset;

	// Build info
	StaticMeshBuildFlag flags; // Build flags

};

class StaticMesh : public Resource
{
public:
	StaticMesh();
	~StaticMesh();

	void createBuildData() override;
	void createBuildData(gfx::GraphicDevice* device, RenderData* data) override;
	void destroyBuildData() override;
	void createRenderData(gfx::GraphicDevice* device, const BuildData* data) override;
	void destroyRenderData(gfx::GraphicDevice* device) override;
	ResourceArchive* createResourceArchive() override;

	uint32_t getVertexCount() const;
	uint32_t getIndexCount() const;

	// Create mesh
	static StaticMesh* createInterleaved(const gfx::VertexAttributeState& state, const void* vertices, uint32_t vertexCount, gfx::IndexFormat indexFormat, const void* indices, uint32_t indexCount);
	static StaticMesh* createInterleaved(const gfx::VertexAttributeState& state, const void* vertices, uint32_t vertexCount);

	// Bind buffers
	void bind(gfx::CommandList* cmd) const;
	void bindVertex(gfx::CommandList* cmd) const;
	void bindIndex(gfx::CommandList* cmd) const;

	// Draw commands
	void draw(gfx::CommandList* cmd, uint32_t instanceCount = 1) const;
	void drawIndexed(gfx::CommandList* cmd, uint32_t instanceCount = 1) const;
	void drawIndirect(gfx::CommandList* cmd, uint32_t instanceCount = 1) const;
	void drawIndexedIndirect(gfx::CommandList* cmd, uint32_t instanceCount = 1) const;

};

};