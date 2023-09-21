#include <Aka/Resource/Archive/ArchiveStaticMesh.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveStaticMesh::ArchiveStaticMesh() : 
	ArchiveStaticMesh(AssetID::Invalid)
{
}
ArchiveStaticMesh::ArchiveStaticMesh(AssetID id) : 
	Archive(AssetType::StaticMesh, id, getLatestVersion()),
	batches{}
{
}

ArchiveLoadResult ArchiveStaticMesh::load_internal(ArchiveLoadContext& _context, BinaryArchive& _archive)
{
	uint32_t batchCount = _archive.read<uint32_t>();
	for (uint32_t i = 0; i < batchCount; i++)
	{
		this->batches.append(ArchiveBatch(_archive.read<AssetID>()));
	}

	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveStaticMesh::save_internal(ArchiveSaveContext& _context, BinaryArchive& _archive)
{
	_archive.write<uint32_t>((uint32_t)this->batches.size());
	for (uint32_t i = 0; i < this->batches.size(); i++)
	{
		_archive.write<AssetID>(this->batches[i].id());
	}

	return ArchiveSaveResult::Success;
}

ArchiveLoadResult ArchiveStaticMesh::load_dependency(ArchiveLoadContext& _context)
{
	for (size_t i = 0; i < this->batches.size(); i++)
	{
		ArchiveLoadResult res = this->batches[i].load(_context);
		if (res != ArchiveLoadResult::Success)
			return res;
	}
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveStaticMesh::save_dependency(ArchiveSaveContext& _context)
{
	for (size_t i = 0; i < this->batches.size(); i++)
	{
		ArchiveSaveResult res = this->batches[i].save(_context);
		if (res != ArchiveSaveResult::Success)
			return res;
	}
	return ArchiveSaveResult::Success;
}

void ArchiveStaticMesh::copyFrom(const Archive* _archive)
{
	AKA_ASSERT(_archive->id() == id(), "Invalid id");
	AKA_ASSERT(_archive->type() == type(), "Invalid type");
	AKA_ASSERT(_archive->version() == version(), "Invalid version");

	const ArchiveStaticMesh* archive = reinterpret_cast<const ArchiveStaticMesh*>(_archive);
	*this = *archive;
}

}