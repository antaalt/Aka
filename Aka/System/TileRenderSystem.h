#pragma once

#include "../Core/System.h"
#include "../Graphic/Shader.h"

namespace aka {

class TileSystem : public System
{
public:
	TileSystem(World* world);

	void draw(Batch &batch) override;
};

}
