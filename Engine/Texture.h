#pragma once

#include <stdint.h>

namespace app {

using TextureID = uint32_t;

class Texture
{
public:
	Texture();
	Texture(const Texture&) = delete;
	const Texture& operator=(const Texture&) = delete;
	~Texture();

	void create(uint32_t width, uint32_t height, const void* data);
	void destroy();

	void bind() const;

	TextureID getID();
private:
	TextureID m_textureID;
};

}
