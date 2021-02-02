#pragma once

#include <Aka.h>
#include <map>

#include "Resources.h"
#include "Level.h"
#include "../GUI/GUINode.h"

namespace aka {

class Game : public Application
{
public:
	Game();
	void initialize() override;
	void destroy() override;
	void frame() override;
	void update(Time::Unit deltaTime) override;
	void render() override;
	bool running() override;
private:
	// UI
	GUI m_gui;
	// Resources
	Resources m_resources;
	// Rendering
	uint32_t m_drawCall = 0;
	Batch m_batch;
	Framebuffer::Ptr m_framebuffer;
	// Entity
	World m_world;
	Entity* m_cameraEntity;
	// Levels
	std::string m_currentLevel;
	WorldMap m_map;
};

}

