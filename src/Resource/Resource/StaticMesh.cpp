#include <Aka/Resource/Resource/StaticMesh.hpp>

#include <Aka/Resource/Archive/ArchiveStaticMesh.hpp>
#include <Aka/Resource/Archive/ArchiveBatch.hpp>
#include <Aka/Resource/Archive/ArchiveGeometry.hpp>
#include <Aka/Resource/AssetLibrary.hpp>
#include <Aka/Renderer/Renderer.hpp>

namespace aka {

gfx::VertexBufferLayout StaticVertex::getState()
{
	gfx::VertexBufferLayout attributes{};
	attributes.add(gfx::VertexSemantic::Position, gfx::VertexFormat::Float, gfx::VertexType::Vec3);
	attributes.add(gfx::VertexSemantic::Normal, gfx::VertexFormat::Float, gfx::VertexType::Vec3);
	attributes.add(gfx::VertexSemantic::TexCoord0, gfx::VertexFormat::Float, gfx::VertexType::Vec2);
	attributes.add(gfx::VertexSemantic::Color0, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	return attributes;
}

struct MaterialUniformBuffer {
	color4f color;
};

StaticMesh::StaticMesh() :
	Resource(ResourceType::StaticMesh)
{
}

StaticMesh::StaticMesh(AssetID _id, const String& _name) :
	Resource(ResourceType::StaticMesh, _id, _name)
{
}
StaticMesh::~StaticMesh()
{
	AKA_ASSERT(m_batches.size() == 0, "Batches should have been cleared.");
}

void StaticMesh::fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer)
{
	const ArchiveStaticMesh& meshArchive = _context.getArchive<ArchiveStaticMesh>(getID());
	Vector<StaticVertex> vertices;
	Vector<uint32_t> indices;
	for (AssetID batchID : meshArchive.batches)
	{
		const ArchiveBatch& batch = _context.getArchive<ArchiveBatch>(batchID);
		const ArchiveGeometry& geometryArchive = _context.getArchive<ArchiveGeometry>(batch.geometry);
		AKA_ASSERT(geometryArchive.skeleton == AssetID::Invalid, "");
		// Material
		ResourceHandle<Material> material = _context.getAssetLibrary()->load<Material>(batch.material, _context, _renderer);
		
		m_batches.append(StaticMeshBatch{
			(uint32_t)(vertices.size() * sizeof(StaticVertex)),
			(uint32_t)(indices.size() * sizeof(uint32_t)),
			(uint32_t)geometryArchive.indices.size(),
			material
		});
		vertices.reserve(vertices.size() + geometryArchive.staticVertices.size());
		for (const ArchiveStaticVertex& archiveVertex : geometryArchive.staticVertices)
		{
			StaticVertex vertex;
			vertex.position = point3f(archiveVertex.position[0], archiveVertex.position[1], archiveVertex.position[2]);
			vertex.normal = norm3f(archiveVertex.normal[0], archiveVertex.normal[1], archiveVertex.normal[2]);
			vertex.uv = uv2f(archiveVertex.uv[0], archiveVertex.uv[1]);
			vertex.color = color4f(archiveVertex.color[0], archiveVertex.color[1], archiveVertex.color[2], archiveVertex.color[3]);
			vertices.append(vertex);
		}
		indices.append(geometryArchive.indices);
		m_bounds.include(geometryArchive.bounds);
	}
	m_indexFormat = gfx::IndexFormat::UnsignedInt;
	m_gfxVertexBufferHandle = _renderer->allocateGeometryVertex(vertices.data(), sizeof(StaticVertex) * vertices.size());
	m_gfxIndexBufferHandle = _renderer->allocateGeometryIndex(indices.data(), sizeof(uint32_t) * indices.size());
}

void StaticMesh::toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer)
{
	AKA_NOT_IMPLEMENTED;
	for (StaticMeshBatch batchID : m_batches)
	{
		AssetID batchAssetID = AssetID::Invalid; // somehow get this.
		AssetID geometryAssetID = AssetID::Invalid; // somehow get this.
		AssetID materialAssetID = AssetID::Invalid; // somehow get this.
		AssetID albedoAssetID = AssetID::Invalid; // somehow get this.
		AssetID normalAssetID = AssetID::Invalid; // somehow get this.
		ArchiveBatch batch(batchAssetID);
		batch.geometry = geometryAssetID;
		batch.material = materialAssetID;
		ArchiveMaterial material(materialAssetID);
		material.albedo = albedoAssetID;
		material.normal = normalAssetID;
		ArchiveGeometry geometry(geometryAssetID);
		geometry.bounds;
		geometry.staticVertices; // Load from buffer
		geometry.indices; // Load from buffer

		_context.addArchive<ArchiveBatch>(batch.id(), batch);
		_context.addArchive<ArchiveMaterial>(material.id(), material);
		_context.addArchive<ArchiveGeometry>(geometry.id(), geometry);

		_context.getAssetLibrary()->save<Texture>(albedoAssetID, _context, _renderer);
		_context.getAssetLibrary()->save<Texture>(normalAssetID, _context, _renderer);
	}
}

void StaticMesh::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
	_renderer->deallocate(m_gfxVertexBufferHandle);
	_renderer->deallocate(m_gfxIndexBufferHandle);
	m_batches.clear();
}

}