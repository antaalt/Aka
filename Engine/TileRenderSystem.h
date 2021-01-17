#pragma once

#include "System.h"
#include "Shader.h"

namespace aka {

class TileSystem : public System
{
public:
	TileSystem(World* world);

	void render(GraphicBackend& backend, Batch &batch) override;
};

}
