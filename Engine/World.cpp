#include "World.h"

#include <nlohmann/json.hpp>
//#include <filesystem>

#include "Graphic.h"

namespace app {

const Level::Layer* Level::getLayer(const std::string& name) const
{
	for (const Level::Layer& layer : layers)
		if (layer.layer->name == name)
			return &layer;
	return nullptr;
}

Level Level::load(const World &world, const Path& path)
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
		std::string name = jsonLayer["name"];
		layer.layer = world.getLayer(name);
		ASSERT(layer.layer->name == name, "");
		layer.gridCellCount = vec2u(jsonLayer["gridCellsX"], jsonLayer["gridCellsY"]);
		layer.gridCellSize = vec2u(jsonLayer["gridCellWidth"], jsonLayer["gridCellHeight"]);
		layer.offset = vec2i(jsonLayer["offsetX"], jsonLayer["offsetY"]);
		switch (layer.layer->type)
		{
		case World::LayerType::Tile:
			layer.tileset = world.getTileset(jsonLayer["tileset"]);
			ASSERT(jsonLayer["arrayMode"].get<int>() == 0, "Only 1D array supported");
			for (const nlohmann::json& jsonData : jsonLayer["data"])
				layer.data.push_back(jsonData);
			break;
		case World::LayerType::Entity:
			for (const nlohmann::json& jsonData : jsonLayer["entities"])
			{
				Entity entity;
				entity.entity = world.getEntity(jsonData["name"]);
				entity.position = vec2u(jsonData["x"], jsonData["y"]);
				entity.size = vec2u(jsonData["width"], jsonData["height"]);
				layer.entities.push_back(entity);
			}
			break;
		case World::LayerType::Grid:
			break;
		}
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

const World::Entity* World::getEntity(const std::string& name) const
{
	for (const World::Entity& entity : entities)
		if (entity.name == name)
			return &entity;
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
	const nlohmann::json& jsonTilesets = json["tilesets"];
	for (const nlohmann::json& jsonTileset : jsonTilesets)
	{
		Tileset tileset;
		tileset.name = jsonTileset["label"];
		std::string imagePath = jsonTileset["path"];
		tileset.image = Image::load(Path(relativePath + imagePath));
		tileset.tileCount = vec2u(jsonTileset["tileWidth"], jsonTileset["tileHeight"]);
		ASSERT(tileset.image.width % tileset.tileCount.x == 0, "");
		ASSERT(tileset.image.height % tileset.tileCount.y == 0, "");
		tileset.tileSize = vec2u(tileset.image.width / tileset.tileCount.x, tileset.image.height / tileset.tileCount.y);
		world.tilesets.push_back(tileset);
	}
	const nlohmann::json& jsonEntities = json["entities"];
	for (const nlohmann::json& jsonEntity : jsonEntities)
	{
		Entity entity;
		entity.name = jsonEntity["name"];
		std::string imagePath = jsonEntity["texture"];
		entity.image = Image::load(Path(relativePath + imagePath));
		entity.origin = vec2u(jsonEntity["origin"]["x"], jsonEntity["origin"]["y"]);
		entity.size = vec2u(jsonEntity["size"]["x"], jsonEntity["size"]["y"]);
		world.entities.push_back(entity);
	}
	const nlohmann::json& jsonLayers = json["layers"];
	for (const nlohmann::json& jsonLayer : jsonLayers)
	{
		std::string definition = jsonLayer["definition"];
		Layer layer;
		layer.name = jsonLayer["name"];
		layer.gridSize.x = jsonLayer["gridSize"]["x"];
		layer.gridSize.y = jsonLayer["gridSize"]["y"];
		if (definition == "tile")
		{
			layer.type = LayerType::Tile;
			layer.tileSet = world.getTileset(jsonLayer["defaultTileset"]);
		}
		else if (definition == "grid")
		{
			layer.type = LayerType::Grid;
			layer.tileSet = nullptr;
		}
		else if (definition == "entity")
		{
			layer.type = LayerType::Entity;
			layer.tileSet = nullptr;
		}
		world.layers.push_back(layer);
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