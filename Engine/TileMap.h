#pragma once

#include "Component.h"
#include "Geometry.h"
#include "Texture.h"

namespace aka {

class TileMapSystem;

struct TileMap : public Component
{
	friend class TileMapSystem;

	TileMap();
	TileMap(const vec2u& gridCount, const vec2u& gridSize, Texture::Ptr texture);

	vec2u gridCount;
	vec2u gridSize;
	Texture::Ptr texture;
private:
	uint32_t vao = 0;
};

}
