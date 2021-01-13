#pragma once

#include <vector>

#include "System.h"
#include "Image.h"
#include "Geometry.h"

namespace app {

struct World
{
	enum class LayerType {
		Tile,
		Grid,
		Entity
	};

	struct Tileset
	{
		std::string name;
		Image image;
		vec2u tileSize;
		vec2u tileCount;
	};

	struct Entity
	{
		std::string name;
		vec2u size;
		vec2u origin;
		Image image;
	};

	struct Layer
	{
		std::string name;
		LayerType type;
		vec2u gridSize;
		const Tileset *tileSet;
	};

	std::string ogmoVersion;
	std::string name;
	std::vector<Layer> layers;
	std::vector<Tileset> tilesets;
	std::vector<Entity> entities;

	const Tileset* getTileset(const std::string& name) const;
	const Entity* getEntity(const std::string& name) const;
	const Layer* getLayer(const std::string& name) const;

	static World load(const Path& path);
};

struct Level
{
	struct Entity
	{
		const World::Entity* entity;
		vec2u position;
		vec2u size;
	};

	struct Layer
	{
		const World::Layer* layer;

		vec2i offset;
		vec2u gridCellSize;
		vec2u gridCellCount;

		const World::Tileset *tileset;
		std::vector<int32_t> data; // index of sprite in corresponding tileset

		std::vector<Entity> entities;

		int32_t getTileIndex(uint32_t tileX, uint32_t tileY) const;
		uint32_t getTileWidth() const;
		uint32_t getTileHeight() const;
	};

	std::string ogmoVersion;
	vec2u size;
	vec2i offset;
	std::vector<Layer> layers;
	World* world;

	const Layer* getLayer(const std::string& name) const;

	static Level load(const World& world, const Path& path);
};

}