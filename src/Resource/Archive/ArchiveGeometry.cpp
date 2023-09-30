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
	vertices{},
	indices{},
	bounds()
{
}
ArchiveParseResult ArchiveGeometry::parse(BinaryArchive& _archive)
{
	// Bounds
	_archive.parse(this->bounds.min);
	_archive.parse(this->bounds.max);

	_archive.parse<uint32_t>(this->indices);
	_archive.parse<ArchiveStaticVertex>(this->vertices);

	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveGeometry::load_dependency(ArchiveLoadContext& _context)
{
	return ArchiveParseResult::Success;
}

}