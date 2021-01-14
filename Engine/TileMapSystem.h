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

	void create() override;
	void destroy() override;

	void update() override;
	void render(GraphicBackend& backend) override;
private:
	Shader m_shader;
};

}