#include <Aka/Resource/Archive/ArchiveSkeleton.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveSkeleton::ArchiveSkeleton() :
	ArchiveSkeleton(AssetID::Invalid)
{
}
ArchiveSkeleton::ArchiveSkeleton(AssetID id) :
	Archive(AssetType::Skeleton, id)
{
}
ArchiveParseResult ArchiveSkeleton::parse(BinaryArchive& _archive)
{
	_archive.parse(rootBoneIndex);
	_archive.parse<ArchiveSkeletalBone>(this->bones, [](BinaryArchive& archive, ArchiveSkeletalBone& bone) {
		archive.parse(bone.parentIndex);
		archive.parse(bone.name);
		archive.parse(bone.offset);
	});
	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveSkeleton::load_dependency(ArchiveLoadContext& _context)
{
	return ArchiveParseResult::Success;
}

};