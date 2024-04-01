#include <Aka/Resource/Archive/ArchiveBatch.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>
#include <Aka/Resource/Archive/ArchiveGeometry.hpp>

namespace aka {

ArchiveBatch::ArchiveBatch() : 
	ArchiveBatch(AssetID::Invalid)
{
}
ArchiveBatch::ArchiveBatch(AssetID id) : 
	Archive(AssetType::Batch, id), 
	material(AssetID::Invalid), 
	geometry(AssetID::Invalid)
{
}
ArchiveParseResult ArchiveBatch::parse(BinaryArchive& _archive)
{
	_archive.parse<AssetID>(this->material);
	_archive.parse<AssetID>(this->geometry);
	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveBatch::load_dependency(ArchiveLoadContext& _context)
{
	_context.addArchive<ArchiveMaterial>(this->material);
	_context.addArchive<ArchiveGeometry>(this->geometry);
	return ArchiveParseResult::Success;
}

}