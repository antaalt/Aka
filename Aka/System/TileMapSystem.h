#pragma once

#include "../Core/System.h"
#include "../Graphic/Shader.h"

namespace aka {

class TileMapSystem : public System
{
public:
	TileMapSystem(World* world);

	void draw(Batch &batch) override;
};

}