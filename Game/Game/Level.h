#pragma once

#include "Resources.h"
#include "OgmoWorld.h"

#include <Core/ECS/Entity.h>

#include <string>

namespace aka {

// Each level has multiple exit collider 
// When loading a level, preload other levels accessibles
struct Level
{
	// Rendering layer
	struct Layer
	{
		Texture::Ptr atlas;
		std::vector<int> tileID;
	};
	// Door to access other levels
	struct Door
	{
		std::string name;
	};

	uint32_t width;
	uint32_t height;
	Layer foreground;
	Layer playerGround;
	Layer background;
	Texture::Ptr backgroundTexture;
	std::vector<Door> doors;

	std::vector<Entity*> entities;
};

class WorldMap
{
public:
	WorldMap(World& world, Resources& resources);

	// Ask for a level
	Level* getLevel(const std::string& level);


	void deleteLevel(const std::string& level);

	void loadLevel(const std::string& level);
private:
	World& m_world;
	Resources& m_resources;
	OgmoWorld m_ogmoWorld;
	std::map<std::string, Level*> m_levels;
};

}