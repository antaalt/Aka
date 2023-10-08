#include <Aka/Resource/Resource/SkeletalMesh.hpp>

#include <Aka/Resource/AssetLibrary.hpp>
#include <Aka/Renderer/Renderer.hpp>

namespace aka {

gfx::VertexBufferLayout SkeletalVertex::getState()
{
	gfx::VertexBufferLayout attributes{};
	attributes.add(gfx::VertexSemantic::Position, gfx::VertexFormat::Float, gfx::VertexType::Vec3);
	attributes.add(gfx::VertexSemantic::Normal, gfx::VertexFormat::Float, gfx::VertexType::Vec3);
	attributes.add(gfx::VertexSemantic::TexCoord0, gfx::VertexFormat::Float, gfx::VertexType::Vec2);
	attributes.add(gfx::VertexSemantic::Color0, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	attributes.add(gfx::VertexSemantic::BlendIndice0, gfx::VertexFormat::UnsignedInt, gfx::VertexType::Vec4);
	attributes.add(gfx::VertexSemantic::BlendWeight0, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	return attributes;
}

SkeletalMesh::SkeletalMesh() :
	Resource(ResourceType::SkeletalMesh)
{
}

SkeletalMesh::SkeletalMesh(AssetID _id, const String& _name) :
	Resource(ResourceType::SkeletalMesh, _id, _name)
{
}
SkeletalMesh::~SkeletalMesh()
{
	AKA_ASSERT(m_batches.size() == 0, "Batches should have been cleared.");
}

void SkeletalMesh::fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer)
{
	const ArchiveSkeletalMesh& meshArchive = _context.getArchive<ArchiveSkeletalMesh>(getID());
	Vector<SkeletalVertex> vertices;
	Vector<uint32_t> indices;
	for (AssetID batchID : meshArchive.batches)
	{
		const ArchiveBatch& batch = _context.getArchive<ArchiveBatch>(batchID);
		const ArchiveGeometry& geometryArchive = _context.getArchive<ArchiveGeometry>(batch.geometry);
		AKA_ASSERT(asBool(geometryArchive.flags & ArchiveGeometryFlags::IsSkeletal), "");

		// Material
		ResourceHandle<Material> material = _context.getAssetLibrary()->load<Material>(batch.material, _context, _renderer);

		m_batches.append(SkeletalMeshBatch{
			(uint32_t)(vertices.size() * sizeof(SkeletalVertex)),
			(uint32_t)(indices.size() * sizeof(uint32_t)),
			(uint32_t)geometryArchive.indices.size(),
			(uint32_t)m_bones.size(),
			material
			});
		vertices.reserve(vertices.size() + geometryArchive.skeletalVertices.size());
		for (const ArchiveSkeletalVertex& archiveVertex : geometryArchive.skeletalVertices)
		{
			SkeletalVertex vertex;
			vertex.position = point3f(archiveVertex.position[0], archiveVertex.position[1], archiveVertex.position[2]);
			vertex.normal = norm3f(archiveVertex.normal[0], archiveVertex.normal[1], archiveVertex.normal[2]);
			vertex.uv = uv2f(archiveVertex.uv[0], archiveVertex.uv[1]);
			vertex.color = color4f(archiveVertex.color[0], archiveVertex.color[1], archiveVertex.color[2], archiveVertex.color[3]);
			for (uint32_t i = 0; i < SkeletalVertex::MaxBoneInfluence; i++)
			{
				vertex.boneIndex[i] = archiveVertex.boneIndex[i];
				vertex.weights[i] = archiveVertex.weights[i];
			}
			vertices.append(vertex);
		}
		indices.append(geometryArchive.indices);
		uint32_t boneID = 0;
		for (const ArchiveSkeletalBone& archiveBone : geometryArchive.skeletalBones)
		{
			SkeletalMeshBone bone;
			for (uint32_t i = 0; i < 4; i++)
			{
				for (uint32_t j = 0; j < 4; j++)
				{
					bone.offset[i][j] = archiveBone.offset[i][j];
				}
			}
			m_bones.append(bone);
		}
		m_bounds.include(geometryArchive.bounds);
	}
	m_indexFormat = gfx::IndexFormat::UnsignedInt;
	m_gfxBonesBufferHandle = _renderer->allocateGeometryData(m_bones.data(), sizeof(SkeletalMeshBone) * m_bones.size());
	m_gfxVertexBufferHandle = _renderer->allocateGeometryVertex(vertices.data(), sizeof(SkeletalVertex) * vertices.size());
	m_gfxIndexBufferHandle = _renderer->allocateGeometryIndex(indices.data(), sizeof(uint32_t) * indices.size());
}

void SkeletalMesh::toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer)
{
	AKA_NOT_IMPLEMENTED;
	for (SkeletalMeshBatch batchID : m_batches)
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
		geometry.skeletalVertices; // Load from buffer
		geometry.indices; // Load from buffer

		_context.addArchive<ArchiveBatch>(batch.id(), batch);
		_context.addArchive<ArchiveMaterial>(material.id(), material);
		_context.addArchive<ArchiveGeometry>(geometry.id(), geometry);

		_context.getAssetLibrary()->save<Texture>(albedoAssetID, _context, _renderer);
		_context.getAssetLibrary()->save<Texture>(normalAssetID, _context, _renderer);
	}
}

void SkeletalMesh::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
	_renderer->deallocate(m_gfxBonesBufferHandle);
	_renderer->deallocate(m_gfxVertexBufferHandle);
	_renderer->deallocate(m_gfxIndexBufferHandle);
	m_batches.clear();
}

}