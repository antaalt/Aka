#include "World.h"

#include <nlohmann/json.hpp>
//#include <filesystem>

#include "Platform.h"

namespace app {

const Level::Layer* Level::getLayer(const std::string& name) const
{
	for (const Level::Layer& layer : layers)
		if (layer.name == name)
			return &layer;
	return nullptr;
}

Level Level::load(const Path& path)
{
	Level level;
	const nlohmann::json json = nlohmann::json::parse(Asset::loadString(path));
	level.size.x = json["width"];
	level.size.y = json["height"];
	level.offset.x = json["offsetX"];
	level.offset.y = json["offsetY"];
	const nlohmann::json& jsonLayers = json["layers"];
	for (const nlohmann::json& jsonLayer : jsonLayers)
	{
		Layer layer;
		layer.name = jsonLayer["name"];
		layer.gridCellCount = vec2u(jsonLayer["gridCellsX"], jsonLayer["gridCellsY"]);
		layer.gridCellSize = vec2u(jsonLayer["gridCellWidth"], jsonLayer["gridCellHeight"]);
		layer.offset = vec2i(jsonLayer["offsetX"], jsonLayer["offsetY"]);
		layer.tileset = jsonLayer["tileset"];
		ASSERT(jsonLayer["arrayMode"].get<int>() == 0, "Only 1D array supported");
		for (const nlohmann::json& jsonData : jsonLayer["data"])
			layer.data.push_back(jsonData);
		level.layers.push_back(layer);
	}
	return level;
}

const World::Tileset* World::getTileset(const std::string& name) const
{
	for (const World::Tileset &tileset : tilesets)
		if (tileset.name == name)
			return &tileset;
	return nullptr;
}

const World::Layer* World::getLayer(const std::string& name) const
{
	for (const World::Layer& layer : layers)
		if (layer.name == name)
			return &layer;
	return nullptr;
}

World World::load(const Path& path)
{
	World world;
	const std::string relativePath = path.str().substr(0, path.str().find_last_of('/') + 1);
	const nlohmann::json json = nlohmann::json::parse(Asset::loadString(path));
	const nlohmann::json& jsonLayers = json["layers"];
	for (const nlohmann::json& jsonLayer : jsonLayers)
	{
		Layer layer;
		layer.name = jsonLayer["name"];
		layer.gridSize.x = jsonLayer["gridSize"]["x"];
		layer.gridSize.y = jsonLayer["gridSize"]["y"];
		layer.tileSet = jsonLayer["defaultTileset"];
		world.layers.push_back(layer);
	}
	const nlohmann::json& jsonTilesets = json["tilesets"];
	for (const nlohmann::json& jsonTileset : jsonTilesets)
	{
		Tileset tileset;
		tileset.name = jsonTileset["label"];
		std::string imagePath = jsonTileset["path"];
		tileset.image = Image::load(Path(relativePath + imagePath));
		world.tilesets.push_back(tileset);
	}

	/*const std::string levelPath = json["levelPaths"][0];
	// check the level path for levels.
	namespace fs = std::filesystem;
	for (const fs::directory_entry& p : fs::directory_iterator(relativePath))
	{		
		std::string currentFile = path.str().substr(path.str().find_last_of('/') + 1);
		std::string file = p.path().string().substr(p.path().string().find_last_of('/') + 1);
		if (file == currentFile)
			continue;
		Level::load(relativePath + file);
	}*/
	return world;
}

int32_t Level::Layer::getTileIndex(uint32_t tileX, uint32_t tileY) const
{
	return data[tileY * gridCellSize.y + tileX];
}

uint32_t Level::Layer::getTileWidth() const
{
	return gridCellSize.x;
}

uint32_t Level::Layer::getTileHeight() const
{
	return gridCellSize.y;
}

}