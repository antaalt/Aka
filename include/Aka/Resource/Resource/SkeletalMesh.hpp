#pragma once 

#include <Aka/Resource/Archive/ArchiveSkeletalMesh.hpp>
#include <Aka/Resource/Resource/Material.hpp>

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Renderer/Renderer.hpp>

namespace aka {

// https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation
// https://wickedengine.net/2017/09/09/skinning-in-compute-shader/
struct SkeletalVertex
{
	point3f position;
	norm3f normal;
	uv2f uv;
	color4f color;

	static const uint32_t MaxBoneInfluence = 4;
	static const uint32_t InvalidBoneIndex = (uint32_t)-1;
	uint32_t boneIndex[MaxBoneInfluence];
	float weights[MaxBoneInfluence];

	static gfx::VertexBufferLayout getState();
};

struct SkeletalMeshBone
{
	mat4f offset; // matrix transform from model space to bone space
};

struct SkeletalMeshBatch
{
	uint32_t vertexOffset;
	uint32_t indexOffset;
	uint32_t indexCount;
	uint32_t boneOffset;
	ResourceHandle<Material> material;
};

class SkeletalMesh : public Resource {
public:
	SkeletalMesh();
	SkeletalMesh(AssetID _id, const String& _name);
	~SkeletalMesh();
private:
	void fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer) override;
	void toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer) override;
	void destroy_internal(AssetLibrary* _library, Renderer* _renderer) override;

public:
	GeometryBufferHandle getVertexBufferHandle() const { return m_gfxVertexBufferHandle; }
	GeometryBufferHandle getIndexBufferHandle() const { return m_gfxIndexBufferHandle; }
	GeometryBufferHandle getBoneBufferHandle() const { return m_gfxBonesBufferHandle; }
	uint32_t getBatchCount() const { return (uint32_t)m_batches.size(); }
	const SkeletalMeshBatch& getBatch(uint32_t index) const { return m_batches[index]; }
	const Vector<SkeletalMeshBatch>& getBatches() const { return m_batches; }
	gfx::IndexFormat getIndexFormat() const { return m_indexFormat; }
	aabbox<> getBounds() const { return m_bounds; }

private:
	GeometryBufferHandle m_gfxVertexBufferHandle;
	GeometryBufferHandle m_gfxIndexBufferHandle;
	GeometryBufferHandle m_gfxBonesBufferHandle;
	gfx::IndexFormat m_indexFormat;
	aabbox<> m_bounds;
	Vector<SkeletalMeshBone> m_bones;
	Vector<SkeletalMeshBatch> m_batches;
};


}