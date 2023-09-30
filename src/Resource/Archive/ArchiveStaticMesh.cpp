#include <Aka/Resource/Archive/ArchiveStaticMesh.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveStaticMesh::ArchiveStaticMesh() : 
	ArchiveStaticMesh(AssetID::Invalid)
{
}
ArchiveStaticMesh::ArchiveStaticMesh(AssetID id) : 
	Archive(AssetType::StaticMesh, id),
	batches{}
{
}

ArchiveParseResult ArchiveStaticMesh::parse(BinaryArchive& _archive)
{
	_archive.parse<AssetID>(this->batches, [](BinaryArchive& archive, AssetID& assetID) {
		archive.parse(assetID);
	});
	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveStaticMesh::load_dependency(ArchiveLoadContext& _context)
{
	for (AssetID assetID : this->batches)
		_context.addArchive<ArchiveBatch>(assetID);
	return ArchiveParseResult::Success;
}

}