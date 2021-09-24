#if defined(AKA_USE_D3D11)
#pragma once

#include "../D3D11Context.h"

namespace aka {

void blitDepth(const Texture::Ptr& src, const Framebuffer::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter);
void blitColor(const Texture::Ptr& src, const Framebuffer::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter);

};

#endif