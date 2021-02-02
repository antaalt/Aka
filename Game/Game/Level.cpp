#include "Level.h"

#include "OgmoWorld.h"
#include "../Component/Transform2D.h"
#include "../Component/TileMap.h"
#include "../Component/TileLayer.h"
#include "../Component/Collider2D.h"
#include "../Component/Animator.h"
#include "../Component/Coin.h"
#include "../Component/Text.h"
#include "../Component/Player.h"
#include "../Component/Door.h"
#include <Core/Sprite.h>
#include <Core/ECS/World.h>

namespace aka {

WorldMap::WorldMap(World& world, Resources& resources) :
	m_world(world),
	m_resources(resources),
	m_ogmoWorld(OgmoWorld::load(Asset::path("levels/world.ogmo")))
{
}

Level* WorldMap::getLevel(const std::string& level)
{
	auto it = m_levels.find(level);
	if (it == m_levels.end())
		return nullptr;
	return it->second;
}

void WorldMap::deleteLevel(const std::string& levelName)
{
	Level* level = getLevel(levelName);
	if (level != nullptr)
	{
		for (Entity* e : level->entities)
			e->destroy();
		delete level;
		m_levels.erase(levelName);
	}
}

void WorldMap::loadLevel(const std::string &str)
{
	Level* level = new Level;
	auto it = m_levels.insert(std::make_pair(str, level));

	// Load Ogmo level
	Path path = Asset::path("levels/" + str + ".json");
	OgmoLevel ogmoLevel = OgmoLevel::load(m_ogmoWorld, path);
	const OgmoLevel::Layer* foreground = ogmoLevel.getLayer("Foreground");

	// - Size
	//ogmoLevel.offset;
	level->width = ogmoLevel.size.x;
	level->height = ogmoLevel.size.y;

	// - Layers
	std::map<std::string, Texture::Ptr> atlas;
	auto createTileLayer = [&](Level::Layer& layer, const OgmoLevel::Layer* ogmoLayer, int32_t layerDepth) -> Entity* {
		if (ogmoLayer->layer->type != OgmoWorld::LayerType::Tile)
			return nullptr;
		ASSERT(ogmoLayer->tileset->tileSize == ogmoLayer->gridCellSize, "");
		Texture::Ptr texture;
		auto it = atlas.find(ogmoLayer->tileset->name);
		if (it == atlas.end())
		{
			texture = Texture::create(
				ogmoLayer->tileset->image.width, 
				ogmoLayer->tileset->image.height, 
				Texture::Format::Rgba, 
				ogmoLayer->tileset->image.bytes.data(), 
				Sampler::Filter::Nearest
			);
			atlas.insert(std::make_pair(ogmoLayer->tileset->name, texture));
		}
		else
		{
			texture = it->second;
		}
		
		layer.atlas = texture;
		layer.tileID = ogmoLayer->data;

		Entity* entity = m_world.createEntity();
		entity->add<Transform2D>(Transform2D(vec2f(0.f), vec2f(1.f), radianf(0.f)));
		entity->add<TileMap>(TileMap(ogmoLayer->tileset->tileCount, ogmoLayer->tileset->tileSize, texture));
		entity->add<TileLayer>(TileLayer(vec2f(0.f), ogmoLayer->gridCellCount, ogmoLayer->gridCellSize, color4f(1.f), ogmoLayer->data, layerDepth));
		return entity;
	};
	// - Layers
	level->entities.push_back(createTileLayer(level->background, ogmoLevel.getLayer("Background"), -1));
	level->entities.push_back(createTileLayer(level->playerGround, ogmoLevel.getLayer("Playerground"), 0));
	level->entities.push_back(createTileLayer(level->foreground, ogmoLevel.getLayer("Foreground"), 1));

	// - Background texture
	std::vector<uint8_t> data;
	data.resize(level->width * level->height * 4);
	memset(data.data(), 0xffffffff, sizeof(data.size()));
	level->backgroundTexture = Texture::create(level->width, level->height, Texture::Format::Rgba, data.data(), Sampler::Filter::Nearest);


	{
		// - Colliders
		Sprite::Animation animation;
		animation.name = "default";
		Image image = Image::load(Asset::path("textures/debug/collider.png"));
		animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
		Sprite* sprite = m_resources.sprite.create("Collider", new Sprite());
		sprite->animations.push_back(animation);
	}
	{
		// - Coins
		Sprite::Animation animation;
		animation.name = "picked";
		std::vector<Path> frames = {
			Asset::path("textures/interact/coin01.png"),
			Asset::path("textures/interact/coin02.png"),
			Asset::path("textures/interact/coin03.png")
		};
		for (Path path : frames)
		{
			Image image = Image::load(path);
			animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(100)));
		}
		Sprite* sprite = m_resources.sprite.create("Coin", new Sprite());
		sprite->animations.push_back(animation);
		Texture::Ptr tex = animation.frames[0].texture;
		animation.frames.clear();
		animation.name = "idle";
		animation.frames.push_back(Sprite::Frame::create(tex, Time::Unit::milliseconds(500)));
		sprite->animations.push_back(animation);
	}
	{
		// - Player
		// TODO move initialisation in SpriteAnimator & parse ase sprite directly ?
		// TODO load gif ?
		Sprite::Animation animation;
		animation.name = "idle";
		std::vector<Path> frames = {
			Asset::path("textures/player/player01.png"),
			Asset::path("textures/player/player02.png"),
			Asset::path("textures/player/player03.png")
		};
		for (Path path : frames)
		{
			Image image = Image::load(path);
			animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
		}
		Sprite* sprite = m_resources.sprite.create("Player", new Sprite());
		sprite->animations.push_back(animation);
	}
	const OgmoLevel::Layer* layer = ogmoLevel.getLayer("Colliders");
	for (const OgmoLevel::Entity& entity : layer->entities)
	{
		if (entity.entity->name == "Collider")
		{
			Entity* e = m_world.createEntity();
			e->add<Transform2D>(Transform2D(vec2f((float)entity.position.x, (float)(layer->getHeight() - entity.position.y - entity.size.y)), vec2f(entity.size) / 16.f, radianf(0.f)));
			e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f)));
			//e->add<Animator>(Animator(m_sprites.back().get(), 1));
			level->entities.push_back(e);
		}
		else if (entity.entity->name == "Coin")
		{
			Entity* e = m_world.createEntity();
			e->add<Transform2D>(Transform2D(vec2f((float)entity.position.x, (float)(layer->getHeight() - entity.position.y - entity.size.y)), vec2f(entity.size) / 16.f, radianf(0)));
			e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f), 0.1f, 0.1f));
			e->add<Animator>(Animator(m_resources.sprite.get("Coin"), 1))->play("idle");
			e->add<Coin>(Coin());
			level->entities.push_back(e);
		}
		else if (entity.entity->name == "Character")
		{
			// TODO do not load player with level
			Sprite* playerSprite = m_resources.sprite.get("Player");
			Entity* e = m_world.createEntity();
			e->add<Transform2D>(Transform2D(vec2f(80, 224), vec2f(1.f), radianf(0)));
			e->add<Animator>(Animator(playerSprite, 1))->play("idle");
			e->add<RigidBody2D>(RigidBody2D(1.f));
			e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f((float)playerSprite->animations[0].frames[0].width, (float)playerSprite->animations[0].frames[0].height), 0.1f, 0.1f));

			Player* player = e->add<Player>(Player());
			player->jump = Control(input::Key::Space);
			player->left = Control(input::Key::Q);
			player->right = Control(input::Key::D);

			e->add<Text>(Text(vec2f(3.f, 17.f), m_resources.font.get("Espera16"), "0", color4f(1.f), 3));
			level->entities.push_back(e);
		}
		else if (entity.entity->name == "LevelDoor")
		{
			const vec2f pos = vec2f((float)entity.position.x, (float)(layer->getHeight() - entity.position.y - entity.size.y));
			const vec2f size = vec2f(entity.size);
			Entity* e = m_world.createEntity();
			e->add<Transform2D>(Transform2D(pos, size, radianf(0)));
			e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(1.f)));
			e->add<Door>(Door());
			level->entities.push_back(e);
			level->doors.emplace_back();
			level->doors.back().name = entity.entity->name; // TODO get level name from ogmo file
		}
		else
		{
			Logger::warn("Ogmo entity not supported : ", entity.entity->name);
		}
	}
}

};