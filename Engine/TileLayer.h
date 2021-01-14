#pragma once

#include "Geometry.h"
#include "Component.h"

#include <vector>

namespace app {

class TileMapSystem;

struct TileLayer : public Component
{
	friend TileMapSystem;

	TileLayer();
	TileLayer(const vec2u &gridCount, const vec2u &gridSize, const color4f &color, const std::vector<int> &tileID, float depth);

	vec2u gridCount;
	vec2u gridSize;
	color4f color;
	std::vector<int> tileID;
	float depth;
private:
	uint32_t tbo = 0;
	uint32_t ubo = 0;
};

};

