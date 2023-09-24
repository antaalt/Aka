#include <Aka/Resource/Importer/Importer.hpp>

namespace aka {

Importer::Importer() :
	m_path(""),
	m_name(""),
	m_flags(ImportFlag::None)
{
}
Importer:: ~Importer()
{
}

void Importer::setFlag(ImportFlag flag)
{
	m_flags |= flag;
}
void Importer::setAssetPath(const AssetPath& path)
{
	m_path = path;
}
void Importer::setName(const String& name)
{
	m_name = name;
}
bool Importer::has(ImportFlag flag) const
{
	return asBool(flag & m_flags);
}
AssetPath Importer::getAssetPath(const char* subPath) const
{
	if (subPath == nullptr)
	{
		return m_path;
	}
	else
	{
		String str = m_path.cstr();
		str.append("/");
		str.append(subPath);
		return AssetPath(str);
	}
}
const String& Importer::getName() const
{
	return m_name;
}

};