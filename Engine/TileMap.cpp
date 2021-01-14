#include "TileMap.h"

namespace aka {
TileMap::TileMap() :
	TileMap(vec2u(0), vec2u(0), nullptr)
{
}
TileMap::TileMap(const vec2u& gridCount, const vec2u& gridSize, Texture* texture) :
	gridCount(gridCount),
	gridSize(gridSize),
	texture(texture)
{
}

}