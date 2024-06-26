#include <Aka/Resource/AssetPath.hpp>

#include <filesystem>
#include <Aka/OS/OS.h>

namespace aka {

AssetPath::AssetPath() :
	m_path(""),
	m_type(AssetPathType::Unknown)
{
}
AssetPath::AssetPath(const char* _path, AssetPathType type) :
	m_path(_path),
	m_type(type)
{
}
AssetPath::AssetPath(const aka::String& _path, AssetPathType type) :
	m_path(_path),
	m_type(type)
{
}
bool AssetPath::operator==(const AssetPath& _path) const
{
	return m_path == _path.m_path && m_type == _path.m_type;
}
bool AssetPath::operator!=(const AssetPath& _path) const
{
	return m_path != _path.m_path || m_type != _path.m_type;
}
AssetPathType AssetPath::getType() const
{
	return m_type;
}
aka::Path AssetPath::getAbsolutePath() const
{
	return OS::cwd() + AssetPath::getAssetPathFromCwd(m_type) + m_path;
}

const aka::Path& AssetPath::getRawPath() const
{
	return m_path;
}

const char* AssetPath::cstr() const
{
	return m_path.cstr();
}

size_t AssetPath::size() const
{
	return m_path.size();
}

aka::Path AssetPath::getAssetPathFromCwd(AssetPathType type)
{
	// Current working directory need to be correctly setup at root of project.
	switch (type)
	{
	case aka::AssetPathType::Common:
		return "./lib/Aka/asset/";
	case aka::AssetPathType::Custom:
		return "./asset/";
	case aka::AssetPathType::Cooked:
		return "./asset/.cooked/";
	default:
		AKA_ASSERT(false, "Invalid asset path type");
		return OS::temp();
	}
}

};