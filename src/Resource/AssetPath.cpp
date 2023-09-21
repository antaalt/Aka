#include <Aka/Resource/AssetPath.hpp>

namespace aka {

AssetPath::AssetPath() :
	m_path("")
{
}
AssetPath::AssetPath(const char* _path) : 
	m_path(_path)
{
}
AssetPath::AssetPath(const aka::String& _path) :
	m_path(_path)
{
}
bool AssetPath::operator==(const AssetPath& _path) const
{
	return m_path == _path.m_path;
}
bool AssetPath::operator!=(const AssetPath& _path) const
{
	return m_path != _path.m_path;
}
aka::Path AssetPath::getAbsolutePath() const
{
	return AssetPath::getAssetPath() + m_path;
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

aka::Path AssetPath::getAssetPath()
{
	// HOLY THAT IS SO DIRTY NEED TO FIX THIS ASAP
	return "../../../asset/library/";
}

};