#include "Game.h"

#include "Component/Transform2D.h"
#include "Component/Collider2D.h"
#include "Component/Camera2D.h"
#include "Component/Text.h"
#include "Component/TileMap.h"
#include "Component/TileLayer.h"
#include "Component/Player.h"
#include "Component/Coin.h"
#include "OgmoWorld.h"
#include "System/PhysicSystem.h"
#include "System/AnimatorSystem.h"
#include "System/TileMapSystem.h"
#include "System/TileRenderSystem.h"
#include "System/TextRenderSystem.h"
#include "System/CameraSystem.h"
#include "System/CollisionSystem.h"
#include "System/PlayerSystem.h"
#include "System/CoinSystem.h"
#include "System/SoundSystem.h"
#include "GUI/EntityWidget.h"
#include "GUI/ResourcesWidget.h"


namespace aka {

void Game::initialize()
{
	Logger::debug.mute();
	{
		// INI SYSTEMS
		m_world.createSystem<PhysicSystem>();
		m_world.createSystem<CollisionSystem>();
		m_world.createSystem<TileSystem>();
		m_world.createSystem<TileMapSystem>();
		m_world.createSystem<AnimatorSystem>();
		m_world.createSystem<CameraSystem>();
		m_world.createSystem<TextRenderSystem>();
		m_world.createSystem<PlayerSystem>();
		m_world.createSystem<CoinSystem>();
		m_world.createSystem<SoundSystem>();
	}

	{
		// INIT sounds
		Entity *e = m_world.createEntity();
		e->add<SoundInstance>(SoundInstance(Asset::path("sounds/forest.mp3"), true));
	}

	{
		// INIT FRAMEBUFFER
		m_framebuffer = Framebuffer::create(320, 180, Sampler::Filter::Nearest);
	}

	{
		// INIT CAMERA
		m_cameraEntity = m_world.createEntity();
		m_cameraEntity->add<Transform2D>(Transform2D());
		m_cameraEntity->add<Camera2D>(Camera2D(vec2f(0), vec2f(320, 180) ));
	}

	{
		// INIT FONTS
		m_resources.font.create("Espera48", new Font(Asset::path("font/Espera/Espera-Bold.ttf"), 48));
		m_resources.font.create("Espera16", new Font(Asset::path("font/Espera/Espera-Bold.ttf"), 16));
		Font *font = m_resources.font.create("BoldFont48", new Font(Asset::path("font/Theboldfont/theboldfont.ttf"), 48));
		m_textEntity = m_world.createEntity();
		Transform2D* transform = m_textEntity->add<Transform2D>(Transform2D());
		Text* text = m_textEntity->add<Text>(Text());

		text->offset = vec2f(0.f);
		text->color = color4f(1.f);
		text->font = font;
		text->text = "Find them all !";
		text->layer = 2;
		vec2i size = font->size(text->text);
		transform->translate(vec2f((float)((int)m_framebuffer->width() / 2 - size.x / 2), (float)((int)m_framebuffer->height() / 2 - size.y / 2 - 50)));
	}

	{
		// INIT FIXED TEXTURE BACKGROUND
		Image image = Image::load(Asset::path("textures/background/background.png"));
		ASSERT(image.width == m_framebuffer->width(), "incorrect width");
		ASSERT(image.height == m_framebuffer->height(), "incorrect height");

		Sprite::Animation animation;
		animation.name = "default";
		animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
		Sprite *sprite = m_resources.sprite.create("Background", new Sprite());
		sprite->animations.push_back(animation);

		m_backgroundEntity = m_world.createEntity();
		m_backgroundEntity->add<Animator>(Animator(sprite, -2))->play("default");
	}

	{
		// INIT world
		OgmoWorld ogmoWorld = OgmoWorld::load(Asset::path("levels/world.ogmo"));
		OgmoLevel ogmoLevel = OgmoLevel::load(ogmoWorld, Asset::path("levels/level0.json"));
		const OgmoLevel::Layer* foreground = ogmoLevel.getLayer("Foreground");

		// - Layers
		auto createTileLayer = [&](const OgmoLevel::Layer* layer, int32_t layerDepth) -> Entity* {
			if (layer->layer->type != OgmoWorld::LayerType::Tile)
				return nullptr;
			ASSERT(layer->tileset->tileSize == layer->gridCellSize, "");
			Texture::Ptr texture = Texture::create(layer->tileset->image.width, layer->tileset->image.height, Texture::Format::Rgba, layer->tileset->image.bytes.data(), Sampler::Filter::Nearest);
			
			Entity* entity = m_world.createEntity();
			Transform2D* transform = entity->add<Transform2D>(Transform2D(vec2f(0.f), vec2f(1.f), radianf(0.f)));
			TileMap* tileMap = entity->add<TileMap>(TileMap(layer->tileset->tileCount, layer->tileset->tileSize, texture));
			TileLayer* tileLayer = entity->add<TileLayer>(TileLayer(vec2f(0.f), layer->gridCellCount, layer->gridCellSize, color4f(1.f), layer->data, layerDepth));
			return entity;
		};
		createTileLayer(ogmoLevel.getLayer("Background"), -1);
		createTileLayer(ogmoLevel.getLayer("Playerground"), 0);
		createTileLayer(ogmoLevel.getLayer("Foreground"), 1);

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
			Entity* collider = m_world.createEntity();
			if (entity.entity->name == "Collider")
			{
				collider->add<Transform2D>(Transform2D(vec2f((float)entity.position.x, (float)(layer->getHeight() - entity.position.y - entity.size.y)), vec2f(entity.size) / 16.f, radianf(0.f)));
				collider->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f)));
				//collider->add<Animator>(Animator(m_sprites.back().get(), 1));
			}
			else if (entity.entity->name == "Coin")
			{
				Entity* e = m_world.createEntity();
				e->add<Transform2D>(Transform2D(vec2f((float)entity.position.x, (float)(layer->getHeight() - entity.position.y - entity.size.y)), vec2f(entity.size) / 16.f, radianf(0)));
				e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f), 0.1f, 0.1f));
				e->add<Animator>(Animator(m_resources.sprite.get("Coin"), 1))->play("idle");
				e->add<Coin>(Coin());
			}
			else if (entity.entity->name == "Character")
			{
				Sprite* playerSprite = m_resources.sprite.get("Player");
				m_playerEntity = m_world.createEntity();
				m_playerEntity->add<Transform2D>(Transform2D(vec2f(80, 224), vec2f(1.f), radianf(0)));
				m_playerEntity->add<Animator>(Animator(playerSprite, 1))->play("idle");
				m_playerEntity->add<RigidBody2D>(RigidBody2D(1.f));
				m_playerEntity->add<Collider2D>(Collider2D(vec2f(0.f), vec2f((float)playerSprite->animations[0].frames[0].width, (float)playerSprite->animations[0].frames[0].height), 0.1f, 0.1f));

				Player* player = m_playerEntity->add<Player>(Player());
				player->jump = Control(input::Key::Space);
				player->left = Control(input::Key::Q);
				player->right = Control(input::Key::D);

				m_playerEntity->add<Text>(Text(vec2f(3.f, 17.f), m_resources.font.get("Espera16"), "0", color4f(1.f), 3));
			}
			else
			{
				Logger::warn("Entity not supported : ", entity.entity->name);
			}
		}
	}

	{
		PlatformBackend::setLimits(m_framebuffer->width(), m_framebuffer->height(), 0, 0);
	}

	{
		m_gui.add(new EntityWidget);
		m_gui.add(new ResourcesWidget);
		m_gui.initialize();
	}

	{
		// Initialize everything
		m_world.create();
	}
}

void Game::destroy()
{
	m_gui.destroy();
}

void Game::frame()
{
	m_gui.frame();
}
// Need a component movable -> take inputs and update movements
// Component hurtable
// collision -> if hurtable and collider has component damage
void Game::update(Time::Unit deltaTime)
{
	if (input::pressed(input::Key::F1))
	{
		GraphicBackend::screenshot("./output.jpg");
		Logger::info("Screenshot taken.");
	}
	if (!m_gui.focused() && input::down(input::Key::H))
	{
		m_gui.setVisible(!m_gui.isVisible());
	}
	// Update world after moving manually objects
	m_world.update(deltaTime);
}

void Game::render()
{
	{
		// Render to framebuffer
		Camera2D* camera = m_cameraEntity->get<Camera2D>();
		mat4f view = mat4f::inverse(mat4f(
			col4f(1.f, 0.f, 0.f, 0.f),
			col4f(0.f, 1.f, 0.f, 0.f),
			col4f(0.f, 0.f, 1.f, 0.f),
			col4f(camera->position.x, camera->position.y, 0.f, 1.f)
		));
		mat4f projection = mat4f::orthographic(0.f, static_cast<float>(m_framebuffer->height()), 0.f, static_cast<float>(m_framebuffer->width()), -1.f, 1.f);
		m_framebuffer->clear(1.f, 0.63f, 0.f, 1.f); 
		m_world.draw(m_batch);
		m_batch.render(m_framebuffer, view, projection);
		m_drawCall = m_batch.count();
		m_batch.clear();
	}

	{
		// Blit to backbuffer
		GraphicBackend::backbuffer()->clear(0.f, 0.f, 0.f, 1.f);
		mat4f view = mat4f::identity();
		mat4f projection = mat4f::orthographic(0.f, static_cast<float>(GraphicBackend::backbuffer()->height()), 0.f, static_cast<float>(GraphicBackend::backbuffer()->width()), -1.f, 1.f);
		m_batch.draw(mat3f::scale(vec2f((float)GraphicBackend::backbuffer()->width(), (float)GraphicBackend::backbuffer()->height())), Batch::Rect(vec2f(0), vec2f(1.f), m_framebuffer->attachment(Framebuffer::AttachmentType::Color0), 0));
		m_batch.render(GraphicBackend::backbuffer(), view, projection);
		m_batch.clear();
	}

	{
		// Rendering imgui
		m_gui.draw(m_world, m_resources);
		//ImGui::ShowDemoWindow();
		m_gui.render();
	}

}

bool Game::running()
{
	return !input::pressed(input::Key::Escape);
}

}