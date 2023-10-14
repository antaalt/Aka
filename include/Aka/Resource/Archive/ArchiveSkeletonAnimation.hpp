#pragma once 

#include <Aka/Resource/Archive/Archive.hpp>

namespace aka {

struct ArchiveSkeletalBonePosition
{
	point3f position;
	float timestamp;
};
struct ArchiveSkeletalBoneRotation
{
	quatf orientation;
	float timestamp;
};
struct ArchiveSkeletalBoneScale
{
	vec3f scale;
	float timestamp;
};

enum class ArchiveSkeletalMeshBehaviour : uint32_t {};

struct ArchiveSkeletonBoneAnimation
{
	uint32_t boneIndex;
	ArchiveSkeletalMeshBehaviour behaviour;
	Vector<ArchiveSkeletalBonePosition> positions;
	Vector<ArchiveSkeletalBoneRotation> rotations;
	Vector<ArchiveSkeletalBoneScale> scales;
};

struct ArchiveSkeletonAnimation : Archive
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveSkeletonAnimation();
	explicit ArchiveSkeletonAnimation(AssetID id);

	AssetID skeleton;
	String name;
	float durationInTick;
	float tickPerSeconds;
	Vector<ArchiveSkeletonBoneAnimation> bones; // per anim
protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

};