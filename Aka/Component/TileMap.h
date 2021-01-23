#pragma once

#include "../Core/Geometry.h"
#include "../Core/ECS/Component.h"
#include "../Graphic/Texture.h"

namespace aka {

struct TileMap : public Component
{
	TileMap();
	TileMap(const vec2u& gridCount, const vec2u& gridSize, Texture::Ptr texture);

	vec2u gridCount;
	vec2u gridSize;
	Texture::Ptr texture;
};

}
