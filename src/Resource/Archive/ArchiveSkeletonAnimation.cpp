#include <Aka/Resource/Archive/ArchiveSkeletonAnimation.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveSkeletonAnimation::ArchiveSkeletonAnimation() :
	ArchiveSkeletonAnimation(AssetID::Invalid)
{
}
ArchiveSkeletonAnimation::ArchiveSkeletonAnimation(AssetID id) :
	Archive(AssetType::SkeletonAnimation, id)
{
}
ArchiveParseResult ArchiveSkeletonAnimation::parse(BinaryArchive& _archive)
{
	_archive.parse(skeleton);
	_archive.parse(name);
	_archive.parse(durationInTick);
	_archive.parse(tickPerSeconds);
	_archive.parse<ArchiveSkeletonBoneAnimation>(this->bones, [](BinaryArchive& archive, ArchiveSkeletonBoneAnimation& bone) {
		archive.parse(bone.boneIndex);
		archive.parse(bone.behaviour);
		archive.parse<ArchiveSkeletalBonePosition>(bone.positions, [](BinaryArchive& archive, ArchiveSkeletalBonePosition& position) {
			archive.parse(position.position);
			archive.parse(position.timestamp);
		});
		archive.parse<ArchiveSkeletalBoneRotation>(bone.rotations, [](BinaryArchive& archive, ArchiveSkeletalBoneRotation& position) {
			archive.parse(position.orientation);
			archive.parse(position.timestamp);
		});
		archive.parse<ArchiveSkeletalBoneScale>(bone.scales, [](BinaryArchive& archive, ArchiveSkeletalBoneScale& position) {
			archive.parse(position.scale);
			archive.parse(position.timestamp);
		});
	});
	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveSkeletonAnimation::load_dependency(ArchiveLoadContext& _context)
{
	return ArchiveParseResult::Success;
}

};