#include <Aka/Resource/Importer/FontImporter.hpp>

#include <Aka/OS/Image.h>
#include <Aka/OS/Archive.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Resource/Resource/Font.hpp>
#include <Aka/Graphic/Texture.h>

namespace aka {

ImportResult import(AssetLibrary * _library, const Path & path)
{
	AKA_NOT_IMPLEMENTED;
	return ImportResult::Failed;
}
ImportResult import(AssetLibrary * _library, const Blob & blob)
{
	AKA_NOT_IMPLEMENTED;
	return ImportResult::Failed;
}

};