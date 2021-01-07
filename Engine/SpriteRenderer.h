#pragma once

#include "Texture.h"
#include "Geometry.h"

namespace app {

class SpriteRenderer
{
public:
	virtual void create() = 0;
	virtual void destroy() = 0;

	void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);
	virtual void render(const Texture& texture, const vec2f& position, const vec2f& size, radianf rotate, const color3f& color) = 0;
protected:
	int32_t m_x, m_y;
	uint32_t m_width, m_height;
};

};
