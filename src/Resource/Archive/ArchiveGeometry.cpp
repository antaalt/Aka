#include <Aka/Resource/Archive/ArchiveGeometry.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveGeometry::ArchiveGeometry() : 
	ArchiveGeometry(AssetID::Invalid)
{
}
ArchiveGeometry::ArchiveGeometry(AssetID id) :
	Archive(AssetType::Geometry, id),
	flags(ArchiveGeometryFlags::None),
	bounds(),
	staticVertices{},
	skeletalVertices{},
	indices{}
{
}
ArchiveParseResult ArchiveGeometry::parse(BinaryArchive& _archive)
{
	_archive.parse(this->flags);

	_archive.parse(this->bounds.min);
	_archive.parse(this->bounds.min);
	_archive.parse(this->bounds.max);

	_archive.parse<uint32_t>(this->indices);
	if (asBool(this->flags & ArchiveGeometryFlags::IsSkeletal))
	{
		_archive.parse<ArchiveSkeletalVertex>(this->skeletalVertices);
		_archive.parse<ArchiveSkeletalBone>(this->skeletalBones);
	}
	else
	{
		_archive.parse<ArchiveStaticVertex>(this->staticVertices);
	}

	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveGeometry::load_dependency(ArchiveLoadContext& _context)
{
	return ArchiveParseResult::Success;
}

}