#include <Aka/Resource/Archive/ArchiveFont.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveFont::ArchiveFont() :
	ArchiveFont(AssetID::Invalid)
{
}
ArchiveFont::ArchiveFont(AssetID id) :
	Archive(AssetType::Font, id)
{
}
ArchiveParseResult ArchiveFont::parse(BinaryArchive& _archive)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveFont::load_dependency(ArchiveLoadContext& _context)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveParseResult::Success;
}

}