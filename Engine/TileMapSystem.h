#pragma once

#include "System.h"
#include "Shader.h"
#include "TileMap.h"
#include "TileLayer.h"

namespace app {

class TileMapSystem : public System
{
public:
	void create() override;
	void destroy() override;

	void update() override;
	void render(GraphicBackend& backend) override;
public:
	bool valid(Entity* entity) override;
private:
	Shader m_shader;
};

}