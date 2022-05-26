#include <Aka/Resource/Importer/FontImporter.h>

#include <Aka/OS/Image.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/OS/Archive.h>
#include <Aka/Resource/Asset.h>
#include <Aka/Resource/Resource/Font.h>
#include <Aka/OS/OS.h>


namespace aka {

bool FontImporter::import(const Path& path, std::function<bool(Resource* resource)>&& callback)
{
	Font* font = new Font;
	font->createBuildData();
	FontBuildData* buildData = reinterpret_cast<FontBuildData*>(font->getBuildData());

	buildData->height = 48;
	if (OS::File::read(path, &buildData->ttf))
	{
		return callback(font);
	}
	else
	{
		return false;
	}
}

};