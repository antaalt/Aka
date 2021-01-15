#include "OgmoWorld.h"

#include <nlohmann/json.hpp>
//#include <filesystem>

#include "Debug.h"

namespace aka {

const OgmoLevel::Layer* OgmoLevel::getLayer(const std::string& name) const
{
	for (const OgmoLevel::Layer& layer : layers)
		if (layer.layer->name == name)
			return &layer;
	return nullptr;
}

OgmoLevel OgmoLevel::load(const OgmoWorld &world, const Path& path)
{
	OgmoLevel level;
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
		case OgmoWorld::LayerType::Tile:
			layer.tileset = world.getTileset(jsonLayer["tileset"]);
			ASSERT(jsonLayer["arrayMode"].get<int>() == 0, "Only 1D array supported");
			for (const nlohmann::json& jsonData : jsonLayer["data"])
				layer.data.push_back(jsonData);
			break;
		case OgmoWorld::LayerType::Entity:
			for (const nlohmann::json& jsonData : jsonLayer["entities"])
			{
				Entity entity;
				entity.entity = world.getEntity(jsonData["name"]);
				entity.position = vec2u(jsonData["x"], jsonData["y"]);
				entity.size = vec2u(jsonData["width"], jsonData["height"]);
				layer.entities.push_back(entity);
			}
			break;
		case OgmoWorld::LayerType::Grid:
			break;
		}
		level.layers.push_back(layer);
	}
	return level;
}

const OgmoWorld::Tileset* OgmoWorld::getTileset(const std::string& name) const
{
	for (const OgmoWorld::Tileset &tileset : tilesets)
		if (tileset.name == name)
			return &tileset;
	return nullptr;
}

const OgmoWorld::Entity* OgmoWorld::getEntity(const std::string& name) const
{
	for (const OgmoWorld::Entity& entity : entities)
		if (entity.name == name)
			return &entity;
	return nullptr;
}

const OgmoWorld::Layer* OgmoWorld::getLayer(const std::string& name) const
{
	for (const OgmoWorld::Layer& layer : layers)
		if (layer.name == name)
			return &layer;
	return nullptr;
}

OgmoWorld OgmoWorld::load(const Path& path)
{
	OgmoWorld world;
	const std::string relativePath = path.str().substr(0, path.str().find_last_of('/') + 1);
	const nlohmann::json json = nlohmann::json::parse(Asset::loadString(path));
	const nlohmann::json& jsonTilesets = json["tilesets"];
	for (const nlohmann::json& jsonTileset : jsonTilesets)
	{
		Tileset tileset;
		tileset.name = jsonTileset["label"];
		std::string imagePath = jsonTileset["path"];
		tileset.image = Image::load(Path(relativePath + imagePath));
		tileset.tileSize = vec2u(jsonTileset["tileWidth"], jsonTileset["tileHeight"]);
		tileset.tileCount = vec2u(tileset.image.width / tileset.tileSize.x, tileset.image.height / tileset.tileSize.y);
		ASSERT(tileset.image.width % tileset.tileCount.x == 0, "");
		ASSERT(tileset.image.height % tileset.tileCount.y == 0, "");
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
		OgmoLevel::load(relativePath + file);
	}*/
	return world;
}

int32_t OgmoLevel::Layer::getTileIndex(uint32_t tileX, uint32_t tileY) const
{
	return data[tileY * gridCellSize.y + tileX];
}

uint32_t OgmoLevel::Layer::getWidth() const
{
	return gridCellSize.x * gridCellCount.x;
}

uint32_t OgmoLevel::Layer::getHeight() const
{
	return gridCellSize.y * gridCellCount.y;
}

uint32_t OgmoLevel::Layer::getTileWidth() const
{
	return gridCellSize.x;
}

uint32_t OgmoLevel::Layer::getTileHeight() const
{
	return gridCellSize.y;
}

}