#include <Aka/Resource/Resource/SkeletalMesh.hpp>


#include <Aka/Resource/Archive/ArchiveSkeletalMesh.hpp>
#include <Aka/Resource/Archive/ArchiveBatch.hpp>
#include <Aka/Resource/Archive/ArchiveGeometry.hpp>
#include <Aka/Resource/Archive/ArchiveMaterial.hpp>
#include <Aka/Resource/Archive/ArchiveSkeleton.hpp>
#include <Aka/Resource/Archive/ArchiveSkeletonAnimation.hpp>
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

void SkeletalMeshBoneAnimation::update(float animationTime)
{
	if (positionKeys.size() == 0 && rotationKeys.size() == 0 && scaleKeys.size() == 0)
		return;
	mat4f translation = interpolatePosition(animationTime);
	mat4f rotation = interpolateRotation(animationTime);
	mat4f scale = interpolateScaling(animationTime);
	localTransform = translation * rotation * scale;
}
// Gets normalized value for Lerp & Slerp
float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
	float scaleFactor = 0.0f;
	float midWayLength = animationTime - lastTimeStamp;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	scaleFactor = midWayLength / framesDiff;
	return scaleFactor;
}
mat4f SkeletalMeshBoneAnimation::interpolatePosition(float animationTime)
{
	if (positionKeys.size() == 1)
		return mat4f::translate(positionKeys[0].position);
	else if (positionKeys.size() == 0)
		return mat4f::identity();

	size_t p0Index = (size_t)-1;
	for (size_t index = 0; index < positionKeys.size() - 1; ++index)
	{
		if (animationTime < positionKeys[index + 1].timeStamp)
		{
			p0Index = index;
			break;
		}
	}
	AKA_ASSERT(p0Index != (size_t)-1, "");
	size_t p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(positionKeys[p0Index].timeStamp, positionKeys[p1Index].timeStamp, animationTime);
	point3f finalPosition = point3f::lerp(positionKeys[p0Index].position, positionKeys[p1Index].position, scaleFactor);
	return mat4f::translate(finalPosition);
}

mat4f SkeletalMeshBoneAnimation::interpolateRotation(float animationTime)
{
	if (rotationKeys.size() == 1)
		return mat4f::rotate(quatf::normalize(rotationKeys[0].orientation));
	else if (rotationKeys.size() == 0)
		return mat4f::identity();

	size_t p0Index = (size_t)-1;
	for (size_t index = 0; index < rotationKeys.size() - 1; ++index)
	{
		if (animationTime < rotationKeys[index + 1].timeStamp)
		{
			p0Index = index;
			break;
		}
	}
	AKA_ASSERT(p0Index != (size_t)-1, "");
	size_t p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(rotationKeys[p0Index].timeStamp, rotationKeys[p1Index].timeStamp, animationTime);
	quatf finalRotation = quatf::slerp(rotationKeys[p0Index].orientation, rotationKeys[p1Index].orientation, scaleFactor);
	finalRotation = quatf::normalize(finalRotation);
	return mat4f::rotate(finalRotation);
}

mat4f SkeletalMeshBoneAnimation::interpolateScaling(float animationTime)
{
	if (scaleKeys.size() == 1)
		return mat4f::scale(scaleKeys[0].scale);
	else if (scaleKeys.size() == 0)
		return mat4f::identity();

	size_t p0Index = (size_t)-1;
	for (size_t index = 0; index < scaleKeys.size() - 1; ++index)
	{
		if (animationTime < scaleKeys[index + 1].timeStamp)
		{
			p0Index = index;
			break;
		}
	}
	AKA_ASSERT(p0Index != (size_t)-1, "");
	size_t p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(scaleKeys[p0Index].timeStamp, scaleKeys[p1Index].timeStamp, animationTime);
	vec3f finalScale = vec3f::lerp(scaleKeys[p0Index].scale, scaleKeys[p1Index].scale, scaleFactor);
	return mat4f::scale(finalScale);
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
	AssetID skeletonID = AssetID::Invalid;
	for (AssetID batchID : meshArchive.batches)
	{
		const ArchiveBatch& batch = _context.getArchive<ArchiveBatch>(batchID);
		const ArchiveGeometry& geometryArchive = _context.getArchive<ArchiveGeometry>(batch.geometry);
		AKA_ASSERT(geometryArchive.skeleton != AssetID::Invalid, "");

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
		m_bounds.include(geometryArchive.bounds);
		indices.append(geometryArchive.indices);
		AKA_ASSERT(skeletonID == AssetID::Invalid || skeletonID == geometryArchive.skeleton, "All batches should share the same skeleton");
		skeletonID = geometryArchive.skeleton;
	}
	const ArchiveSkeleton& skeleton = _context.getArchive<ArchiveSkeleton>(skeletonID);
	m_rootBoneIndex = skeleton.rootBoneIndex;
	for (const ArchiveSkeletalBone& archiveBone : skeleton.bones)
	{
		SkeletalMeshBone bone;
		bone.parentIndex = archiveBone.parentIndex;
		bone.name = archiveBone.name;
		bone.offset = archiveBone.offset;
		m_bones.append(bone);
	}
	m_animations.reserve(meshArchive.animations.size());
	for (AssetID animationID : meshArchive.animations)
	{
		// TODO this is in mesh component instead.
		const ArchiveSkeletonAnimation& archiveAnimation = _context.getArchive<ArchiveSkeletonAnimation>(animationID);

		SkeletalMeshAnimation animation;
		animation.durationInTick = archiveAnimation.durationInTick;
		animation.tickPerSecond = archiveAnimation.tickPerSeconds;
		animation.name = archiveAnimation.name;
		animation.bones.resize(m_bones.size());
		for (const ArchiveSkeletonBoneAnimation& archiveBone : archiveAnimation.bones)
		{
			SkeletalMeshBoneAnimation& bone = animation.bones[archiveBone.boneIndex];
			bone.localTransform = mat4f::identity();
			bone.behaviour = static_cast<SkeletalMeshBehaviour>(archiveBone.behaviour);
			for (size_t i = 0; i < archiveBone.positions.size(); i++)
			{
				bone.positionKeys.append(SkeletalMeshKeyPosition{ archiveBone.positions[i].position, archiveBone.positions[i].timestamp });
			}
			for (size_t i = 0; i < archiveBone.rotations.size(); i++)
			{
				bone.rotationKeys.append(SkeletalMeshKeyRotation{ archiveBone.rotations[i].orientation, archiveBone.rotations[i].timestamp });
			}
			for (size_t i = 0; i < archiveBone.scales.size(); i++)
			{
				bone.scaleKeys.append(SkeletalMeshKeyScale{ archiveBone.scales[i].scale, archiveBone.scales[i].timestamp });
			}
		}
		m_animations.append(animation);
	}
	m_indexFormat = gfx::IndexFormat::UnsignedInt;
	m_gfxVertexBufferHandle = _renderer->allocateGeometryVertex(vertices.data(), sizeof(SkeletalVertex) * vertices.size(), sizeof(SkeletalVertex));
	m_gfxIndexBufferHandle = _renderer->allocateGeometryIndex(indices.data(), sizeof(uint32_t) * indices.size(), sizeof(uint32_t));
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
	_renderer->deallocate(m_gfxVertexBufferHandle);
	_renderer->deallocate(m_gfxIndexBufferHandle);
	m_batches.clear();
}

}