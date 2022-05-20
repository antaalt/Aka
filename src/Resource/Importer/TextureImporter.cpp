#include <Aka/Resource/Importer/TextureImporter.h>

#include <Aka/OS/Image.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Resource/Asset.h>
#include <Aka/Resource/Resource/Texture.h>
#include <Aka/Resource/Archive/TextureArchive.h>
#include <Aka/OS/OS.h>

#include <stb_image.h>

namespace aka {

bool TextureImporter::import(const Path& path, std::function<bool(Resource* resource)>&& callback)
{
	// Generate build data
	Texture* texture = new Texture;
	texture->createBuildData();
	TextureBuildData* buildData = reinterpret_cast<TextureBuildData*>(texture->getBuildData());

	// TODO cubemap
	// Cubemap : convert from envmap ? Need a flag...
	int x, y, c;
	stbi_uc* data = stbi_load(path.cstr(), &x, &y, &c, STBI_default);
	if (data == nullptr)
	{
		float* dataf = stbi_loadf(path.cstr(), &x, &y, &c, STBI_default);
		if (dataf == nullptr)
		{
			delete texture;
			return false;
		}
		buildData->width = x;
		buildData->height = y;
		buildData->channels = c;
		buildData->layers = 1;
		buildData->flags = TextureBuildFlag::ColorSpaceHDR;
		buildData->bytes.append(
			reinterpret_cast<byte_t*>(dataf), 
			reinterpret_cast<byte_t*>(dataf) + x * y * c * sizeof(float)
		);
		stbi_image_free(dataf);
	}
	else
	{
		buildData->width = x;
		buildData->height = y;
		buildData->channels = c;
		buildData->layers = 1;
		buildData->flags = TextureBuildFlag::None;
		buildData->bytes.append(data, data + x * y * c);
		stbi_image_free(data);
	}

	// Callback importer
	return callback(texture);
}

};