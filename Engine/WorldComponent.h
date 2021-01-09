#pragma once

#include "Component.h"
#include "World.h"

namespace app {

class WorldComponent : public Component
{
public:
	void loadWorld(const Path &worldPath);
	void loadLevel(const Path& levelPath);
public:
	void create(GraphicBackend& backend) override;
	void destroy(GraphicBackend& backend) override;

	void update() override;
	void render(GraphicBackend& backend) override;
private:
	World m_world;
	Level m_currentLevel;
};

};