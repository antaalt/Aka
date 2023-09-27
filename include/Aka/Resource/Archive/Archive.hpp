#pragma once

#include <Aka/OS/Path.h>
#include <Aka/OS/Archive.h>
#include <Aka/Resource/Asset.hpp>

#include <set>
#include <map>

namespace aka {

class AssetLibrary;

enum class ArchiveLoadResult {
	Success,

	InvalidMagicWord,
	IncompatibleVersion,
	InvalidAssetID,
	InvalidDependency,

	Failed,
};

enum class ArchiveSaveResult {
	Success,

	InvalidDependency,

	Failed,
};

struct Archive;

using AssetSaveCache = std::set<AssetID>;
using AssetLoadCache = std::map<AssetID, Archive*>;

struct ArchiveLoadContext
{
	ArchiveLoadContext(AssetLibrary* library) : cache{}, library(library) {}

	AssetLoadCache cache;
	AssetLibrary* library;
};

struct ArchiveSaveContext
{
	ArchiveSaveContext(AssetLibrary* library) : cache{}, library(library) {}

	AssetSaveCache cache;
	AssetLibrary* library;
};


using ArchiveVersionType = uint32_t;

enum class ArchiveVersion : ArchiveVersionType
{
	ArchiveCreation = 0,

	Latest = ArchiveCreation
};

struct Archive 
{
	Archive(AssetType _type) : m_type(_type), m_id(AssetID::Invalid) {}
	Archive(AssetType _type, AssetID _id) : m_type(_type), m_id(_id) {}

	ArchiveLoadResult load(ArchiveLoadContext& _context, const Blob& _blob, bool _loadDependency = true);
	ArchiveLoadResult load(ArchiveLoadContext& _context, const AssetPath& _path, bool _loadDependency = true);
	ArchiveLoadResult load(ArchiveLoadContext& _context, bool _loadDependency = true);
	ArchiveSaveResult save(ArchiveSaveContext& _context, Blob& _blob, bool _saveDependency = true);
	ArchiveSaveResult save(ArchiveSaveContext& _context, const AssetPath& _path, bool _saveDependency = true);
	ArchiveSaveResult save(ArchiveSaveContext& _context, bool _saveDependency = true);

	bool validate(AssetLibrary* _library);

	AssetType type() const { return m_type; }
	AssetID id() const { return m_id; }
	ArchiveVersion version() const { return ArchiveVersion::Latest; }

protected:
	static const char* getFileMagicWord(AssetType _type);
	ArchiveLoadResult readHeader(BinaryArchive& _archive);
	ArchiveSaveResult writeHeader(BinaryArchive& _archive);
	virtual ArchiveLoadResult load_internal(ArchiveLoadContext& _context, BinaryArchive& _archive) = 0;
	virtual ArchiveSaveResult save_internal(ArchiveSaveContext& _context, BinaryArchive& _archive) = 0;
	virtual ArchiveLoadResult load_dependency(ArchiveLoadContext& _context) = 0;
	virtual ArchiveSaveResult save_dependency(ArchiveSaveContext& _context) = 0;
	virtual ArchiveVersionType getLatestVersion() const = 0;
	virtual void copyFrom(const Archive* _archive) = 0;
private:
	AssetType m_type;
	AssetID m_id;
};

}