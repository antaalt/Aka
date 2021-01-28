#pragma once

#include <Core/Geometry.h>
#include <Core/ECS/Component.h>

#include <vector>

namespace aka {

struct TileLayer : public Component
{
	TileLayer();
	TileLayer(const vec2f& offset, const vec2u &gridCount, const vec2u &gridSize, const color4f &color, const std::vector<int> &tileID, int32_t layer);

	vec2f offset;
	vec2u gridCount;
	vec2u gridSize;
	color4f color;
	std::vector<int> tileID;
	int32_t layer;
};

};

