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
ArchiveLoadResult ArchiveFont::load_internal(ArchiveLoadContext& _context, BinaryArchive& _archive)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveFont::save_internal(ArchiveSaveContext& _context, BinaryArchive& _archive)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveSaveResult::Success;
}

ArchiveLoadResult ArchiveFont::load_dependency(ArchiveLoadContext& _context)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveFont::save_dependency(ArchiveSaveContext& _context)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveSaveResult::Success;
}
void ArchiveFont::copyFrom(const Archive* _archive)
{
	AKA_ASSERT(_archive->id() == id(), "Invalid id");
	AKA_ASSERT(_archive->type() == type(), "Invalid type");
	AKA_ASSERT(_archive->version() == version(), "Invalid version");

	const ArchiveFont* batch = reinterpret_cast<const ArchiveFont*>(_archive);
	*this = *batch;
}

}