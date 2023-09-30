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

ArchiveLoadContext::ArchiveLoadContext(Archive& _archive, AssetLibrary * library, bool _loadDependency) :
	m_dependencies{},
	m_archive(_archive),
	m_library(library),
	m_loadDependencies(_loadDependency)
{
	m_dependencies.insert(std::make_pair(_archive.id(), &_archive));
}
ArchiveLoadContext::~ArchiveLoadContext() 
{
	for (auto it : m_dependencies)
		if (it.first != m_archive.id())
			delete it.second;
}


ArchiveSaveContext::ArchiveSaveContext(Archive& _archive, AssetLibrary* library, bool _saveDependency) :
	m_dependencies{},
	m_archive(_archive),
	m_library(library),
	m_saveDependency(_saveDependency)
{
	m_dependencies.insert(std::make_pair(_archive.id(), &_archive));
}
ArchiveSaveContext::~ArchiveSaveContext()
{
	for (auto it : m_dependencies)
		if (it.first != m_archive.id())
			delete it.second;
}

ArchiveParseResult Archive::load(ArchiveLoadContext& _context, const Vector<byte_t>& _blob)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	if (_context.hasArchive(id()) && !_context.isMainArchive(id()))
	{
		return ArchiveParseResult::Success; // dependency already loaded
	}
	else
	{
		MemoryReaderStream stream(_blob);
		BinaryArchiveReader archive(stream);

		ArchiveParseResult res = parseHeader(archive);
		if (res != ArchiveParseResult::Success)
			return res;
		res = parse(archive);
	}
	if (_context.shouldLoadDependency())
		return load_dependency(_context);
	else
		return ArchiveParseResult::Success;
}

ArchiveParseResult Archive::load(ArchiveLoadContext& _context, const AssetPath& _path)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	if (_context.hasArchive(id()) && !_context.isMainArchive(id()))
	{
		return ArchiveParseResult::Success; // dependency already loaded
	}
	else
	{
		FileStream stream(_path.getAbsolutePath(), FileMode::Read, FileType::Binary);
		BinaryArchiveReader archive(stream);

		ArchiveParseResult res = parseHeader(archive);
		if (res != ArchiveParseResult::Success)
			return res;
		res = parse(archive);
	}
	if (_context.shouldLoadDependency())
		return load_dependency(_context);
	else
		return ArchiveParseResult::Success;
}

ArchiveParseResult Archive::load(ArchiveLoadContext& _context)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	AssetInfo info = _context.getAssetLibrary()->getAssetInfo(id());
#if 0 // Debug log
	if (_context.cache.find(id()) == _context.cache.end())
		Logger::info(info.path.cstr(), " load from cache ", info.readCounter);
	else
		Logger::info(info.path.cstr(), " read ", info.readCounter++);
#endif
	return load(_context, info.path);
}

ArchiveParseResult Archive::save(ArchiveSaveContext& _context, Vector<byte_t>& _blob)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	MemoryWriterStream stream(_blob);
	BinaryArchiveWriter archive(stream);

	ArchiveParseResult res = parseHeader(archive);
	if (res != ArchiveParseResult::Success)
		return res;
	res = parse(archive);
	if (res != ArchiveParseResult::Success)
		return res;
	return ArchiveParseResult::Success;
}

ArchiveParseResult Archive::save(ArchiveSaveContext& _context, const AssetPath& _path)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	FileStream stream(_path.getAbsolutePath(), FileMode::Write, FileType::Binary);
	BinaryArchiveWriter archive(stream);

	ArchiveParseResult res = parseHeader(archive);
	if (res != ArchiveParseResult::Success)
		return res;
	res = parse(archive);
	if (res != ArchiveParseResult::Success)
		return res;
	return ArchiveParseResult::Success;
}

ArchiveParseResult Archive::save(ArchiveSaveContext& _context)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	AssetInfo& info = _context.getAssetLibrary()->getAssetInfo(id());
#if 0 // Debug log
	if (_context.cache.find(id()) == _context.cache.end())
		Logger::info(info.path.cstr(), " skip bcs of cache ", info.writeCounter);
	else
		Logger::info(info.path.cstr(), " written ", info.writeCounter++);
#endif
	return save(_context, info.path);
}

bool Archive::validate(AssetLibrary* _library)
{
	AKA_ASSERT(id() != AssetID::Invalid, "Invalid AssetID");
	AssetInfo info = _library->getAssetInfo(id());
	FileStream stream(info.path.getAbsolutePath(), FileMode::Read, FileType::Binary);
	BinaryArchiveReader archive(stream);
	return parseHeader(archive) == ArchiveParseResult::Success;
}

const char* Archive::getFileMagicWord(AssetType _type)
{
	return s_magicWord[aka::EnumToIndex(_type)];
}

ArchiveParseResult Archive::parseHeader(BinaryArchive& _archive)
{
	if (!_archive.expectBlob(Archive::getFileMagicWord(type()), 4))
		return ArchiveParseResult::InvalidMagicWord;
	if (!_archive.expect<ArchiveVersion>(version()))
		return ArchiveParseResult::IncompatibleVersion;
	if (!_archive.expect<ArchiveVersionType>(getLatestVersion()))
		return ArchiveParseResult::IncompatibleVersion;
	if (!_archive.expect<AssetID>(id()))
		return ArchiveParseResult::InvalidAssetID;
	return ArchiveParseResult::Success;
}

};