#pragma once

#include "System.h"
#include "Shader.h"

namespace aka {

class TileSystem : public System
{
public:
	TileSystem(World* world);

	void draw(Batch &batch) override;
};

}
