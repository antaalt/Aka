#pragma once

#include "System.h"
#include "Shader.h"

namespace app {

class TileSystem : public System
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
	uint32_t m_vao;
};

}
