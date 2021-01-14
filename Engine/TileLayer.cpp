#include "TileLayer.h"

namespace app {
TileLayer::TileLayer() :
	TileLayer(vec2u(0), vec2u(0), color4f(1.f), std::vector<int>(), 0.f)
{
}
TileLayer::TileLayer(const vec2u& gridCount, const vec2u& gridSize, const color4f& color, const std::vector<int>& tileID, float depth) :
	gridCount(gridCount),
	gridSize(gridSize),
	color(color),
	tileID(tileID), 
	depth(depth)
{
}

}