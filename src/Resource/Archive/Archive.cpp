#include <Aka/Resource/Archive/Archive.hpp>

#include <Aka/Resource/AssetLibrary.hpp>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/OS/Stream/MemoryStream.h>

namespace aka {

constexpr const char* s_magicWord[] = {
	"akag", // MeshGeometry
	"akam", // MeshMaterial
	"akab", // MeshBatch
	"akas", // StaticMesh
	"akad", // DynamicMesh
	"akaa", // Sprite (Animation)
	"akai", // Image
	"akaf", // Font
	"akaa", // Audio
	"akaw", // Scene (World)
};

static_assert(countof(s_magicWord) == EnumCount<AssetType>());

ArchiveLoadResult Archive::load(ArchiveLoadContext& _context, const Vector<byte_t>& _blob, bool _loadDependency)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	auto it = _context.cache.find(id());
	if (it != _context.cache.end())
	{
		copyFrom(it->second);
		return ArchiveLoadResult::Success; // dependency already loaded
	}
	else
	{
		MemoryReaderStream stream(_blob);
		BinaryArchive archive(stream);

		ArchiveLoadResult res = readHeader(archive);
		if (res != ArchiveLoadResult::Success)
			return res;
		res = load_internal(_context, archive);
		_context.cache.insert(std::make_pair(id(), this));
	}
	if (_loadDependency)
		return load_dependency(_context);
	else
		return ArchiveLoadResult::Success;
}

ArchiveLoadResult Archive::load(ArchiveLoadContext& _context, const AssetPath& _path, bool _loadDependency)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	auto it = _context.cache.find(id());
	if (it != _context.cache.end())
	{
		copyFrom(it->second);
		return ArchiveLoadResult::Success; // dependency already loaded
	}
	else
	{
		FileStream stream(_path.getAbsolutePath(), FileMode::Read, FileType::Binary);
		BinaryArchive archive(stream);

		ArchiveLoadResult res = readHeader(archive);
		if (res != ArchiveLoadResult::Success)
			return res;
		res = load_internal(_context, archive);
		_context.cache.insert(std::make_pair(id(), this));
	}
	if (_loadDependency)
		return load_dependency(_context);
	else
		return ArchiveLoadResult::Success;
}

ArchiveLoadResult Archive::load(ArchiveLoadContext& _context, bool _loadDependency)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	AssetInfo info = _context.library->getAssetInfo(id());
#if 0 // Debug log
	if (_context.cache.find(id()) == _context.cache.end())
		Logger::info(info.path.cstr(), " load from cache ", info.readCounter);
	else
		Logger::info(info.path.cstr(), " read ", info.readCounter++);
#endif
	return load(_context, info.path, _loadDependency);
}

ArchiveSaveResult Archive::save(ArchiveSaveContext& _context, Vector<byte_t>& _blob, bool _saveDependency)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	auto it = _context.cache.find(id());
	if (it == _context.cache.end())
	{
		MemoryWriterStream stream(_blob);
		BinaryArchive archive(stream);

		ArchiveSaveResult res = writeHeader(archive);
		if (res != ArchiveSaveResult::Success)
			return res;
		res = save_internal(_context, archive);
		if (res != ArchiveSaveResult::Success)
			return res;
		_context.cache.insert(id());

		if (_saveDependency)
			return save_dependency(_context);
		else
			return ArchiveSaveResult::Success;
	}
	else
	{
		return ArchiveSaveResult::Success; // already written
	}
}

ArchiveSaveResult Archive::save(ArchiveSaveContext& _context, const AssetPath& _path, bool _saveDependency)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	auto it = _context.cache.find(id());
	if (it == _context.cache.end())
	{
		FileStream stream(_path.getAbsolutePath(), FileMode::Write, FileType::Binary);
		BinaryArchive archive(stream);

		ArchiveSaveResult res = writeHeader(archive);
		if (res != ArchiveSaveResult::Success)
			return res;
		res = save_internal(_context, archive);
		if (res != ArchiveSaveResult::Success)
			return res;
		_context.cache.insert(id());
	
		if (_saveDependency)
			return save_dependency(_context);
		else
			return ArchiveSaveResult::Success;
	}
	else
	{
		return ArchiveSaveResult::Success; // already written
	}
}

ArchiveSaveResult Archive::save(ArchiveSaveContext& _context, bool _saveDependency)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	AssetInfo& info = _context.library->getAssetInfo(id());
#if 0 // Debug log
	if (_context.cache.find(id()) == _context.cache.end())
		Logger::info(info.path.cstr(), " skip bcs of cache ", info.writeCounter);
	else
		Logger::info(info.path.cstr(), " written ", info.writeCounter++);
#endif
	return save(_context, info.path, _saveDependency);
}

bool Archive::validate(AssetLibrary* _library)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	AssetInfo info = _library->getAssetInfo(id());
	FileStream stream(info.path.getAbsolutePath(), FileMode::Read, FileType::Binary);
	BinaryArchive archive(stream);
	return readHeader(archive) == ArchiveLoadResult::Success;
}

const char* Archive::getFileMagicWord(AssetType _type)
{
	return s_magicWord[aka::EnumToIndex(_type)];
}

ArchiveLoadResult Archive::readHeader(BinaryArchive& _archive)
{
	const char* magicWord = Archive::getFileMagicWord(type());
	char sign[4];
	_archive.read<char>(sign, 4);
	for (uint32_t i = 0; i < 4; i++)
		if (sign[i] != magicWord[i])
			return ArchiveLoadResult::InvalidMagicWord;
	ArchiveVersion archiveVersion = _archive.read<ArchiveVersion>();
	if (archiveVersion != version())
		return ArchiveLoadResult::IncompatibleVersion;
	ArchiveVersionType archiveSubVersion = _archive.read<ArchiveVersionType>();
	if (archiveSubVersion != getLatestVersion())
		return ArchiveLoadResult::IncompatibleVersion;
	AssetID assetID = _archive.read<AssetID>();
	if (assetID != id() || assetID == AssetID::Invalid)
		return ArchiveLoadResult::InvalidAssetID;
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult Archive::writeHeader(BinaryArchive& _archive)
{
	const char* magicWord = Archive::getFileMagicWord(type());
	_archive.write<char>(magicWord, 4);
	_archive.write<ArchiveVersion>(version());
	_archive.write<ArchiveVersionType>(getLatestVersion());
	_archive.write<AssetID>(id());
	return ArchiveSaveResult::Success;
}

};