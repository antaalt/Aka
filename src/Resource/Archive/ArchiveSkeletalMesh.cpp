#include <Aka/Resource/Archive/ArchiveSkeletalMesh.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/Archive/ArchiveBatch.hpp>
#include <Aka/Resource/Archive/ArchiveSkeletonAnimation.hpp>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveSkeletalMesh::ArchiveSkeletalMesh() :
	ArchiveSkeletalMesh(AssetID::Invalid)
{
}
ArchiveSkeletalMesh::ArchiveSkeletalMesh(AssetID id) :
	Archive(AssetType::StaticMesh, id),
	batches{}
{
}

ArchiveParseResult ArchiveSkeletalMesh::parse(BinaryArchive& _archive)
{
	_archive.parse<AssetID>(this->animations, [](BinaryArchive& archive, AssetID& assetID) {
		archive.parse(assetID);
	});
	_archive.parse<AssetID>(this->batches, [](BinaryArchive& archive, AssetID& assetID) {
		archive.parse(assetID);
	});
	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveSkeletalMesh::load_dependency(ArchiveLoadContext& _context)
{
	for (AssetID assetID : this->batches)
		_context.addArchive<ArchiveBatch>(assetID);
	for (AssetID assetID : this->animations)
		_context.addArchive<ArchiveSkeletonAnimation>(assetID);
	return ArchiveParseResult::Success;
}

}