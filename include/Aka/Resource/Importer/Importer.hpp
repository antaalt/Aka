#pragma once

#include <Aka/OS/Path.h>
#include <Aka/Core/Container/String.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

enum class ImportResult {
	Succeed,

	CouldNotReadFile,

	Failed,
};

enum class ImportFlag
{
	None		= 0,
	Overwrite	= 1 << 0,
	//Constant	= 1 << 1, // hot reloaded import when src is modified, need to store src path & add to a watcher
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ImportFlag);

class Importer {
public:
	Importer();
	virtual ~Importer();

	virtual ImportResult import(AssetLibrary* _library, const Path & path) = 0;
	virtual ImportResult import(AssetLibrary* _library, const Blob& blob) = 0;

public:
	void setFlag(ImportFlag flag);
	void setAssetPath(const AssetPath& path);
	void setName(const String& name);
public:
	bool has(ImportFlag flag) const;
	AssetPath getAssetPath(const char* subPath = nullptr) const;
	const String& getName() const;
private:
	AssetPath m_path;
	String m_name;
	ImportFlag m_flags;
};

}