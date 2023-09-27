#include <Aka/Resource/Archive/ArchiveMaterial.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveMaterial::ArchiveMaterial() : 
	ArchiveMaterial(AssetID::Invalid)
{
}
ArchiveMaterial::ArchiveMaterial(AssetID id) : 
	Archive(AssetType::Material, id),
	flags(ArchiveMaterialFlag::None),
	color(0.f, 0.f, 0.f, 1.f),
	albedo(),
	normal()
{
}
ArchiveLoadResult ArchiveMaterial::load_internal(ArchiveLoadContext& _context, BinaryArchive& _archive)
{
	_archive.read<ArchiveMaterialFlag>(this->flags);
	_archive.read<color4f>(this->color);

	this->albedo = ArchiveImage(_archive.read<AssetID>());
	this->normal = ArchiveImage(_archive.read<AssetID>());

	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveMaterial::save_internal(ArchiveSaveContext& _context, BinaryArchive& _archive)
{
	_archive.write<ArchiveMaterialFlag>(this->flags);
	_archive.write<color4f>(this->color);

	_archive.write<AssetID>(this->albedo.id());
	_archive.write<AssetID>(this->normal.id());
	
	return ArchiveSaveResult::Success;
}

ArchiveLoadResult ArchiveMaterial::load_dependency(ArchiveLoadContext& _context)
{
	ArchiveLoadResult res = this->albedo.load(_context);
	if (res != ArchiveLoadResult::Success)
		return ArchiveLoadResult::InvalidDependency;
	res = this->normal.load(_context);
	if (res != ArchiveLoadResult::Success)
		return ArchiveLoadResult::InvalidDependency;
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveMaterial::save_dependency(ArchiveSaveContext& _context)
{
	ArchiveSaveResult res = this->albedo.save(_context);
	if (res != ArchiveSaveResult::Success)
		return ArchiveSaveResult::InvalidDependency;
	res = this->normal.save(_context);
	if (res != ArchiveSaveResult::Success)
		return ArchiveSaveResult::InvalidDependency;
	return ArchiveSaveResult::Success;
}
void ArchiveMaterial::copyFrom(const Archive* _archive)
{
	AKA_ASSERT(_archive->id() == id(), "Invalid id");
	AKA_ASSERT(_archive->type() == type(), "Invalid type");
	AKA_ASSERT(_archive->version() == version(), "Invalid version");

	const ArchiveMaterial* archive = reinterpret_cast<const ArchiveMaterial*>(_archive);
	*this = *archive;
}

}