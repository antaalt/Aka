#include "WorldComponent.h"

namespace app {

void WorldComponent::loadWorld(const Path& worldPath)
{
	m_world = World::load(worldPath);
}

void WorldComponent::loadLevel(const Path& levelPath)
{
	// TODO destroy currentLevel
	m_currentLevel = Level::load(levelPath);
}

void WorldComponent::create(GraphicBackend& backend)
{
	// create texture for atlas
	// create texture for level
}

void WorldComponent::destroy(GraphicBackend& backend)
{
}

void WorldComponent::update()
{
}

void WorldComponent::render(GraphicBackend& backend)
{
}

};

