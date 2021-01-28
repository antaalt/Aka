#pragma once

#include <vector>

#include "../../OS/Filesystem.h"
#include "../../OS/Image.h"
#include "../Core/Geometry.h"

namespace aka {

struct OgmoWorld
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

	static OgmoWorld load(const Path& path);
};

struct OgmoLevel
{
	struct Entity
	{
		const OgmoWorld::Entity* entity;
		vec2u position;
		vec2u size;
	};

	struct Layer
	{
		const OgmoWorld::Layer* layer;

		vec2i offset;
		vec2u gridCellSize;
		vec2u gridCellCount;

		const OgmoWorld::Tileset *tileset;
		std::vector<int32_t> data; // index of sprite in corresponding tileset

		std::vector<Entity> entities;

		int32_t getTileIndex(uint32_t tileX, uint32_t tileY) const;
		uint32_t getWidth() const;
		uint32_t getHeight() const;
		uint32_t getTileWidth() const;
		uint32_t getTileHeight() const;
	};

	std::string ogmoVersion;
	vec2u size;
	vec2i offset;
	std::vector<Layer> layers;
	OgmoWorld* world;

	const Layer* getLayer(const std::string& name) const;

	static OgmoLevel load(const OgmoWorld& world, const Path& path);
};

}