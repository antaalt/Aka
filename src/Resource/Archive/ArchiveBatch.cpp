#include <Aka/Resource/Archive/ArchiveBatch.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

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
ArchiveLoadResult ArchiveBatch::load_internal(ArchiveLoadContext& _context, BinaryArchive& _archive)
{
	this->material = ArchiveMaterial(_archive.read<AssetID>());
	this->geometry = ArchiveGeometry(_archive.read<AssetID>());
	
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveBatch::save_internal(ArchiveSaveContext& _context, BinaryArchive& _archive)
{
	_archive.write<AssetID>(this->material.id());
	_archive.write<AssetID>(this->geometry.id());

	return ArchiveSaveResult::Success;
}

ArchiveLoadResult ArchiveBatch::load_dependency(ArchiveLoadContext& _context)
{
	ArchiveLoadResult result = this->material.load(_context);
	if (result != ArchiveLoadResult::Success)
		return ArchiveLoadResult::InvalidDependency;
	result = this->geometry.load(_context);
	if (result != ArchiveLoadResult::Success)
		return ArchiveLoadResult::InvalidDependency;
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveBatch::save_dependency(ArchiveSaveContext& _context)
{
	ArchiveSaveResult result = this->material.save(_context);
	if (result != ArchiveSaveResult::Success)
		return ArchiveSaveResult::InvalidDependency;
	result = this->geometry.save(_context);
	if (result != ArchiveSaveResult::Success)
		return ArchiveSaveResult::InvalidDependency;
	return ArchiveSaveResult::Success;
}
void ArchiveBatch::copyFrom(const Archive* _archive)
{
	AKA_ASSERT(_archive->id() == id(), "Invalid id");
	AKA_ASSERT(_archive->type() == type(), "Invalid type");
	AKA_ASSERT(_archive->version() == version(), "Invalid version");

	const ArchiveBatch* batch = reinterpret_cast<const ArchiveBatch*>(_archive);
	*this = *batch;
}

}