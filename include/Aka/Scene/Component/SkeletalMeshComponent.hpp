#pragma once

#include <Aka/Scene/ECS/World.hpp>
#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Resource/Resource/SkeletalMesh.hpp>
#include <Aka/Renderer/Instance.hpp>
#include <Aka/Resource/Archive/ArchiveScene.hpp>

namespace aka {

struct SkeletalMeshComponent
{
	AssetID assetID = AssetID::Invalid;

	uint32_t m_currentAnimation;
	uint32_t m_rootBoneIndex;
	Vector<SkeletalMeshAnimation> m_animations; // TODO do we need this ?
	Vector<SkeletalMeshBone> m_bones; // instantiated bones.

	ResourceHandle<SkeletalMesh> meshHandle;
	InstanceHandle instance = InstanceHandle::Invalid;
};
AKA_DECL_COMPONENT(SkeletalMeshComponent)

template <>
struct ecs::ArchiveComponent<SkeletalMeshComponent>
{
	AssetID assetID;

	void from(const SkeletalMeshComponent& component)
	{
		assetID = component.assetID;
	}
	void to(SkeletalMeshComponent& component) const
	{
		component.assetID = assetID;
	}
	void parse(BinaryArchive& archive)
	{
		archive.parse(assetID);
	}
};

};