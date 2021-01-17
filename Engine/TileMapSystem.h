#pragma once

#include "System.h"
#include "Shader.h"
#include "TileMap.h"
#include "TileLayer.h"

namespace aka {

class TileMapSystem : public System
{
public:
	TileMapSystem(World* world);

	void draw(Batch &batch) override;
};

}