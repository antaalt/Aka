#include "TileLayer.h"

namespace aka {
TileLayer::TileLayer() :
	TileLayer(vec2f(0.f), vec2u(0), vec2u(0), color4f(1.f), std::vector<int>(), 0)
{
}
TileLayer::TileLayer(const vec2f& offset, const vec2u& gridCount, const vec2u& gridSize, const color4f& color, const std::vector<int>& tileID, int32_t layer) :
	offset(offset),
	gridCount(gridCount),
	gridSize(gridSize),
	color(color),
	tileID(tileID), 
	layer(layer)
{
}

}