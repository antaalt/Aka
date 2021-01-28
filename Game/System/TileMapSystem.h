#pragma once

#include <Core/ECS/System.h>
#include <Graphic/Shader.h>

namespace aka {

class TileMapSystem : public System
{
public:
	TileMapSystem(World* world);

	void draw(Batch &batch) override;
};

}