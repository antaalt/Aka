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
ArchiveParseResult ArchiveMaterial::parse(BinaryArchive& _archive)
{
	_archive.parse<ArchiveMaterialFlag>(this->flags);
	_archive.parse(this->color);

	_archive.parse<AssetID>(this->albedo);
	_archive.parse<AssetID>(this->normal);

	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveMaterial::load_dependency(ArchiveLoadContext& _context)
{
	_context.addArchive<ArchiveImage>(this->albedo);
	_context.addArchive<ArchiveImage>(this->normal);
	return ArchiveParseResult::Success;
}

}