#pragma once

#include <vector>

#include "System.h"
#include "Image.h"
#include "Geometry.h"

namespace app {

struct Level
{
	struct Layer
	{
		std::string name;
		vec2i offset;
		vec2u gridCellSize;
		vec2i gridCellPosition;
		std::string tileset;
		std::vector<int32_t> data; // index of sprite in corresponding tileset

		int32_t getTileIndex(uint32_t tileX, uint32_t tileY) const;
		uint32_t getTileWidth() const;
		uint32_t getTileHeight() const;
	};

	std::string ogmoVersion;
	vec2u size;
	vec2i offset;
	std::vector<Layer> layers;

	const Layer* getLayer(const std::string& name) const;

	static Level load(const Path& path);
};

struct World
{
	struct Layer
	{
		std::string name;
		vec2u gridSize;
		std::string tileSet;
	};

	struct Entity
	{

	};

	struct Tileset
	{
		std::string name;
		Image image;
	};

	std::string ogmoVersion;
	std::string name;
	std::vector<Layer> layers;
	std::vector<Tileset> tilesets;

	const Tileset * getTileset(const std::string& name) const;
	const Layer * getLayer(const std::string& name) const;

	static World load(const Path& path);
};

}