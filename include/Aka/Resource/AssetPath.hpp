#pragma once

#include <Aka/OS/Path.h>

namespace aka {

struct AssetPath
{
	AssetPath();
	AssetPath(const char* _path);
	AssetPath(const String& _path);
	AssetPath(const AssetPath&) = default;
	AssetPath(AssetPath&&) = default;
	AssetPath& operator=(const AssetPath&) = default;
	AssetPath& operator=(AssetPath&&) = default;
	~AssetPath() = default;


	bool operator==(const AssetPath& _path) const;
	bool operator!=(const AssetPath& _path) const;

	// Get the absolute path of the data
	Path getAbsolutePath() const;
	// Get the asset path as a raw Path
	const Path& getRawPath() const;

	const char* cstr() const;
	size_t size() const;

	static Path getAssetPath();
private:
	Path m_path;
};

};