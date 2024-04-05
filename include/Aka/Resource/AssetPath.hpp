#pragma once

#include <Aka/OS/Path.h>

#include <iostream>

namespace aka {


// In debug, to support hot reload, we need real asset path for shaders.
//	->
// In release, we should cook everything into a packed file.
//	-> The asset path is then a binary path TBD 


enum class AssetPathType {
	Unknown,

	Common, // Common assets used by aka framework
	Custom, // Custom assets used by application
	Cooked, // Cooked assets that are usable by framework

	First = Common,
	Last = Custom
};

struct AssetPath
{
	AssetPath();
	explicit AssetPath(const char* _path, AssetPathType type = AssetPathType::Custom);
	explicit AssetPath(const String& _path, AssetPathType type = AssetPathType::Custom);
	AssetPath(const AssetPath&) = default;
	AssetPath(AssetPath&&) = default;
	AssetPath& operator=(const AssetPath&) = default;
	AssetPath& operator=(AssetPath&&) = default;
	~AssetPath() = default;


	bool operator==(const AssetPath& _path) const;
	bool operator!=(const AssetPath& _path) const;

	// Get the asset path type
	AssetPathType getType() const;
	// Get the absolute path of the data
	Path getAbsolutePath() const;
	// Get the asset path as a raw Path
	const Path& getRawPath() const;
	// Generate an assetPath from a raw path.
	//static AssetPath fromRawPath(const Path& path);

	const char* cstr() const;
	size_t size() const;

	static Path getAssetPathFromCwd(AssetPathType type);
private:
	AssetPathType m_type;
	Path m_path;
};

inline std::ostream& operator<<(std::ostream& os, const AssetPath& path)
{
	os << path.getRawPath();
	return os;
}

};