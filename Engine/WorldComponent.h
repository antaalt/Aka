#pragma once

#include "Component.h"
#include "OgmoWorld.h"

namespace app {

class WorldComponent : public Component
{
public:
	WorldComponent();
	void loadWorld(const Path &worldPath);
	void loadLevel(const Path& levelPath);
public:
	void create(GraphicBackend& backend) override;
	void destroy(GraphicBackend& backend) override;

	void update() override;
	void render(const Camera2D &camera, GraphicBackend& backend) override;
	void renderLayer(const std::string& name, const Camera2D& camera, GraphicBackend& backend);

	OgmoLevel& getCurrentLevel() { return m_currentLevel; }

private:
	OgmoWorld m_world;
	OgmoLevel m_currentLevel;
private:
	Shader m_shader;
	Texture* m_atlas;
};

};