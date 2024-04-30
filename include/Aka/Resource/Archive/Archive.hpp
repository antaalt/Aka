#pragma once

#include <Aka/OS/Path.h>
#include <Aka/OS/Archive.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Core/Container/HashMap.hpp>

#include <set>
#include <map>

namespace aka {

class AssetLibrary;

enum class ArchiveParseResult {
	Success,

	InvalidMagicWord,
	IncompatibleVersion,
	InvalidAssetID,
	InvalidDependency,
	FileDoesNotExist,

	Failed,
};

const char* getArchiveParseResultString(ArchiveParseResult _result);

struct Archive;

using AssetLoadCache = HashMap<AssetID, Archive*>;

struct ArchiveLoadContext final
{
	ArchiveLoadContext(Archive& _archive, AssetLibrary* library, bool _loadDependency = true);
	ArchiveLoadContext(const ArchiveLoadContext&) = delete;
	ArchiveLoadContext(ArchiveLoadContext&&) = delete;
	ArchiveLoadContext& operator=(const ArchiveLoadContext&) = delete;
	ArchiveLoadContext& operator=(ArchiveLoadContext&&) = delete;
	~ArchiveLoadContext();

	template <typename T> T& addArchive(AssetID assetID);
	template <typename T> T& getArchive(AssetID assetID);
	bool hasArchive(AssetID assetID);
	bool isMainArchive(AssetID assetID);

	bool shouldLoadDependency() const { return m_loadDependencies; }

	AssetLibrary* getAssetLibrary() { return m_library; }
private:
	Archive& m_archive;
	bool m_loadDependencies;
	AssetLoadCache m_dependencies;
	AssetLibrary* m_library;
};

struct ArchiveSaveContext final
{
	ArchiveSaveContext(Archive& _archive, AssetLibrary* library, bool _saveDependency = true);
	ArchiveSaveContext(const ArchiveSaveContext&) = delete;
	ArchiveSaveContext(ArchiveSaveContext&&) = delete;
	ArchiveSaveContext& operator=(const ArchiveSaveContext&) = delete;
	ArchiveSaveContext& operator=(ArchiveSaveContext&&) = delete;
	~ArchiveSaveContext();

	template <typename T> T& addArchive(AssetID assetID, T& archive);
	template <typename T> T& getArchive(AssetID assetID);
	bool hasArchive(AssetID assetID);
	bool isMainArchive(AssetID assetID);

	AssetLibrary* getAssetLibrary() { return m_library; }
private:
	Archive& m_archive;
	bool m_saveDependency;
	AssetLoadCache m_dependencies;
	AssetLibrary* m_library;
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
	virtual ~Archive() {}

	ArchiveParseResult load(ArchiveLoadContext& _context, const Vector<byte_t>& _blob);
	ArchiveParseResult load(ArchiveLoadContext& _context, const AssetPath& _path);
	ArchiveParseResult load(ArchiveLoadContext& _context);
	ArchiveParseResult save(ArchiveSaveContext& _context, Vector<byte_t>& _blob);
	ArchiveParseResult save(ArchiveSaveContext& _context, const AssetPath& _path);
	ArchiveParseResult save(ArchiveSaveContext& _context);

	bool validate(AssetLibrary* _library);

	AssetType type() const { return m_type; }
	AssetID id() const { return m_id; }
	ArchiveVersion version() const { return ArchiveVersion::Latest; }

protected:
	static const char* getFileMagicWord(AssetType _type);
	ArchiveParseResult parseHeader(BinaryArchive& _archive);
	virtual ArchiveParseResult parse(BinaryArchive& _archive) = 0;
	virtual ArchiveParseResult load_dependency(ArchiveLoadContext& _context) = 0;
	virtual ArchiveVersionType getLatestVersion() const = 0;
private:
	AssetType m_type;
	AssetID m_id;
};


template <typename T> 
inline T& ArchiveLoadContext::addArchive(AssetID assetID)
{
	static_assert(std::is_base_of<Archive, T>::value);
	auto it = m_dependencies.find(assetID);
	if (it == m_dependencies.end())
	{
		T* archive = mem::akaNew<T>(AllocatorMemoryType::Object, AllocatorCategory::Assets, assetID);
		if (m_loadDependencies)
		{
			ArchiveParseResult res = archive->load(*this);
			AKA_ASSERT(res == ArchiveParseResult::Success, "Failed to parse archive");
		}
		auto it = m_dependencies.insert(std::make_pair(assetID, archive));
		AKA_ASSERT(it.second, "Failed to insert");;
		return *archive;
	}
	else
	{
		AKA_ASSERT(it->second->type() == T().type(), "Invalid type");
		return reinterpret_cast<T&>(*it->second);
	}
}

inline bool ArchiveLoadContext::hasArchive(AssetID assetID)
{
	auto it = m_dependencies.find(assetID);
	return it != m_dependencies.end();
}

inline bool ArchiveLoadContext::isMainArchive(AssetID assetID)
{
	return assetID == m_archive.id();
}

template <typename T> 
inline T& ArchiveLoadContext::getArchive(AssetID assetID)
{
	static_assert(std::is_base_of<Archive, T>::value);
	auto it = m_dependencies.find(assetID);
	AKA_ASSERT(it != m_dependencies.end(), "Not in cache.");
	AKA_ASSERT(it->second->type() == T().type(), "Invalid type");
	return reinterpret_cast<T&>(*it->second);
}

template <typename T>
inline T& ArchiveSaveContext::addArchive(AssetID assetID, T& _archive)
{
	static_assert(std::is_base_of<Archive, T>::value);
	auto it = m_dependencies.find(assetID);
	if (it == m_dependencies.end())
	{
		T* archive = mem::akaNew<T>(AllocatorMemoryType::Object, AllocatorCategory::Assets, _archive);
		if (m_saveDependency)
		{
			ArchiveParseResult res = archive->save(*this);
			AKA_ASSERT(res == ArchiveParseResult::Success, "Failed to parse archive");
		}
		auto it = m_dependencies.insert(std::make_pair(assetID, archive));
		AKA_ASSERT(it.second, "Failed to insert");;
		return *archive;
	}
	else
	{
		AKA_ASSERT(it->second->type() == T().type(), "Invalid type");
		return reinterpret_cast<T&>(*it->second);
	}
}

inline bool ArchiveSaveContext::hasArchive(AssetID assetID)
{
	auto it = m_dependencies.find(assetID);
	return it != m_dependencies.end();
}

inline bool ArchiveSaveContext::isMainArchive(AssetID assetID)
{
	return assetID == m_archive.id();
}

template <typename T>
inline T& ArchiveSaveContext::getArchive(AssetID assetID)
{
	static_assert(std::is_base_of<Archive, T>::value);
	auto it = m_dependencies.find(assetID);
	AKA_ASSERT(it != m_dependencies.end(), "Not in cache.");
	AKA_ASSERT(it->second->type() == T().type(), "Invalid type");
	return reinterpret_cast<T&>(*it->second);
}

}