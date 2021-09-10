#include <Aka/Graphic/TextureCubeMap.h>

#include <Aka/Graphic/GraphicBackend.h>

namespace aka {

TextureCubeMap::TextureCubeMap(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags) :
	Texture(width, height, 1, TextureType::TextureCubeMap, format, flags)
{
}

TextureCubeMap::~TextureCubeMap()
{
}

TextureCubeMap::Ptr TextureCubeMap::create(
	uint32_t width, uint32_t height, 
	TextureFormat format, TextureFlag flags,
	const void* px, const void* nx,
	const void* py, const void* ny,
	const void* pz, const void* nz
)
{
	return GraphicBackend::createTextureCubeMap(width, height, format, flags, px, nx, py, ny, pz, nz);

}
};