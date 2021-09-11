#include <Aka/Graphic/Texture2D.h>

#include <Aka/Graphic/GraphicBackend.h>

namespace aka {

Texture2D::Texture2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags) :
	Texture(width, height, 1, TextureType::Texture2D, format, flags)
{
}

Texture2D::~Texture2D()
{
}

Texture2D::Ptr Texture2D::create(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data)
{
	return GraphicBackend::createTexture2D(width, height, format, flags, data);
}

void Texture2D::copy(const Texture2D::Ptr& src, const Texture2D::Ptr& dst, uint32_t level)
{
	AKA_ASSERT(src->width() == dst->width() && src->height() == dst->height(), "Incompatible dimensions.");
	Rect rect{ 0, 0, src->width(), src->height() };
	src->copy(dst, rect, rect, level);
}

void Texture2D::copy(const Texture2D::Ptr& src, const Texture2D::Ptr& dst, const Rect& rectSRC, const Rect& rectDST, uint32_t level)
{
	src->copy(dst, rectSRC, rectDST, level);
}

void Texture2D::blit(const Texture2D::Ptr& src, const Texture2D::Ptr& dst, TextureFilter filter, uint32_t level)
{
	src->blit(dst, Rect{ 0, 0, src->width(), src->height() }, Rect{ 0, 0, dst->width(), dst->height() }, filter, level);
}

void Texture2D::blit(const Texture2D::Ptr& src, const Texture2D::Ptr& dst, const Rect& rectSRC, const Rect& rectDST, TextureFilter filter, uint32_t level)
{
	src->blit(dst, rectSRC, rectDST, filter, level);
}

};