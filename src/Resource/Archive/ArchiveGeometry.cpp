#include <Aka/Resource/Archive/ArchiveGeometry.hpp>

#include <Aka/Resource/Archive/ArchiveSkeleton.hpp>
#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveGeometry::ArchiveGeometry() : 
	ArchiveGeometry(AssetID::Invalid)
{
}
ArchiveGeometry::ArchiveGeometry(AssetID id) :
	Archive(AssetType::Geometry, id),
	skeleton(AssetID::Invalid),
	bounds(),
	staticVertices{},
	skeletalVertices{},
	indices{}
{
}
ArchiveParseResult ArchiveGeometry::parse(BinaryArchive& _archive)
{
	_archive.parse(this->skeleton);

	_archive.parse(this->bounds.min);
	_archive.parse(this->bounds.min);
	_archive.parse(this->bounds.max);

	_archive.parse<uint32_t>(this->indices);
	if (this->skeleton != AssetID::Invalid)
	{
		_archive.parse<ArchiveSkeletalVertex>(this->skeletalVertices);
	}
	else
	{
		_archive.parse<ArchiveStaticVertex>(this->staticVertices);
	}

	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveGeometry::load_dependency(ArchiveLoadContext& _context)
{
	if (skeleton != AssetID::Invalid)
		_context.addArchive<ArchiveSkeleton>(skeleton);
	return ArchiveParseResult::Success;
}

}