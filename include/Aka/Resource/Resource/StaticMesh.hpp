#pragma once 

#include <Aka/Resource/Archive/ArchiveStaticMesh.hpp>
#include <Aka/Resource/Resource/Material.hpp>

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Renderer/Renderer.hpp>

namespace aka {

struct StaticVertex
{
	point3f position;
	norm3f normal;
	uv2f uv;
	color4f color;

	static gfx::VertexBufferLayout getState();
};

struct StaticMeshBatch
{
	uint32_t vertexOffset;
	uint32_t indexOffset;
	uint32_t indexCount;
	ResourceHandle<Material> material;
};

class StaticMesh : public Resource {
public:
	StaticMesh();
	StaticMesh(AssetID _id, const String& _name);
	~StaticMesh();
private:
	void fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer) override;
	void toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer) override;
	void destroy_internal(AssetLibrary* _library, Renderer* _renderer) override;

public:
	GeometryBufferHandle getVertexBufferHandle() const { return m_gfxVertexBufferHandle; }
	GeometryBufferHandle getIndexBufferHandle() const { return m_gfxIndexBufferHandle; }
	//gfx::BufferHandle getVertexBuffer() const { return m_gfxVertexBuffer; }
	//gfx::BufferHandle getIndexBuffer() const { return m_gfxIndexBuffer; }
	uint32_t getBatchCount() const { return (uint32_t)m_batches.size(); }
	const StaticMeshBatch& getBatch(uint32_t index) const { return m_batches[index]; }
	const Vector<StaticMeshBatch>& getBatches() const { return m_batches; }
	gfx::IndexFormat getIndexFormat() const { return m_indexFormat; }
	aabbox<> getBounds() const { return m_bounds; }

private:
	// Should be mutualized in a big single geometry buffer with pages.
	GeometryBufferHandle m_gfxVertexBufferHandle;
	GeometryBufferHandle m_gfxIndexBufferHandle;
	//gfx::BufferHandle m_gfxVertexBuffer;
	//gfx::BufferHandle m_gfxIndexBuffer;
	gfx::IndexFormat m_indexFormat;
	aabbox<> m_bounds;
	Vector<StaticMeshBatch> m_batches;
};


}