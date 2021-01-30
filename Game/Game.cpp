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

/*void renderGUI(World &world, Resources &resources)
{
#if defined(USE_IMGUI)
	if (ImGui::Begin("Editor##window"))
	{
		ImVec4 color = ImVec4(236.f / 255.f, 11.f / 255.f, 67.f / 255.f, 1.f);
		if (ImGui::CollapsingHeader("Infos", ImGuiTreeNodeFlags_DefaultOpen))
		{
			uint32_t width, height;
			PlatformBackend::getSize(&width, &height);
			static Device device = Device::getDefault();
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("Resolution : %ux%u", width, height);
			ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
			ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
			//ImGui::Text("Draw call : %u", m_drawCall);
			const char* apiName[] = {
				"OpenGL",
				"DirectX11"
			};
			ImGui::Text("Api : %s", apiName[(int)GraphicBackend::api()]);
			ImGui::Text("Device : %s", device.vendor);

			static bool renderColliders = false;
			if (ImGui::Checkbox("Render colliders", &renderColliders))
			{
				world.each<Collider2D>([&](Entity * entity, Collider2D * collider) {
					if (renderColliders)
					{
						if(!entity->has<Animator>())
							entity->add<Animator>(Animator(resources.sprite.get("Collider"), 2));
					}
					else
					{
						if (entity->has<Animator>() && !entity->has<Player>() && !entity->has<Coin>())
							entity->remove<Animator>();
					}
				});
			}
			static bool vsync = true;
			if (ImGui::Checkbox("Vsync", &vsync))
			{
				GraphicBackend::vsync(vsync);
			}
		}

		if (ImGui::CollapsingHeader("Entities##header", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextColored(color, "Entities");
			static const int componentCount = 12;
			static const char* components[componentCount] = {
				"None",
				"Transform2D",
				"Animator",
				"Collider2D",
				"RigidBody2D",
				"Text",
				"TileMap",
				"TileLayer",
				"Coin",
				"Player",
				"Camera2D",
				"SoundInstance"
			};
			static int currentFilter = 0;
			ImGui::Combo("Filter", &currentFilter, components, componentCount);
			uint32_t index = 0;
			world.each([&](Entity* entity) {
				index++;
				switch (currentFilter)
				{
				case 1: if (!entity->has<Transform2D>()) return; break;
				case 2: if (!entity->has<Animator>()) return; break;
				case 3: if (!entity->has<Collider2D>()) return; break;
				case 4: if (!entity->has<RigidBody2D>()) return; break;
				case 5: if (!entity->has<Text>()) return; break;
				case 6: if (!entity->has<TileMap>()) return; break;
				case 7: if (!entity->has<TileLayer>()) return; break;
				case 8: if (!entity->has<Coin>()) return; break;
				case 9: if (!entity->has<Player>()) return; break;
				case 10: if (!entity->has<Camera2D>()) return; break;
				case 11: if (!entity->has<SoundInstance>()) return; break;
				}
				char buffer[256];
				snprintf(buffer, 256, "Entity %u", index);
				if (ImGui::TreeNode(buffer))
				{
					// --- Transform2D
					if (entity->has<Transform2D>())
					{
						Transform2D* transform = entity->get<Transform2D>();
						snprintf(buffer, 256, "Transform2D##%p", transform);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat2("Position", transform->model[2].data, 3);
							vec2f size = vec2f(transform->model[0].x, transform->model[1].y);
							ImGui::InputFloat2("Size", size.data, 3);
							//ImGui::InputFloat("Rotation", &transform->rotation(), 0.1f, 1.f, 3);

							if (ImGui::Button("Remove")) { entity->remove<Transform2D>(); }

							ImGui::TreePop();
						}
					}
					// --- Animator
					if (entity->has<Animator>())
					{
						Animator* animator = entity->get<Animator>();
						snprintf(buffer, 256, "Animator##%p", animator);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							if (animator->sprite == nullptr)
							{
								static char path[256];
								ImGui::InputText("Path", path, 256);
								if (ImGui::Button("Load sprite"))
								{
									//Sprite::Animation animation;
									//animation.name = "default";
									//Image image = Image::load(Asset::path(path));
									//animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
									//m_sprites.push_back(std::make_shared<Sprite>());
									//m_sprites.back()->animations.push_back(animation);
									//animator->sprite = m_sprites.back().get();
								}
							}
							else
							{
								for (Sprite::Animation& anim : animator->sprite->animations)
								{
									snprintf(buffer, 256, "%s (%llu ms)", anim.name.c_str(), anim.duration().milliseconds());
									if (ImGui::TreeNode(buffer))
									{
										strcpy_s(buffer, anim.name.c_str());
										if (ImGui::InputText("Name", buffer, 256))
											anim.name = buffer;
										int frameID = 0;
										for (Sprite::Frame& frame : anim.frames)
										{
											snprintf(buffer, 256, "Frame %d (%llu ms)", frameID++, frame.duration.milliseconds());
											if (ImGui::TreeNode(buffer))
											{
												float ratio = static_cast<float>(frame.texture->width()) / static_cast<float>(frame.texture->height());
												ImGui::Image((void*)frame.texture->handle().value(), ImVec2(256, 256 * 1 / ratio), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), color);

												ImGui::TreePop();
											}
										}
										ImGui::TreePop();
									}
								}
							}

							ImGui::SliderInt("Animation", reinterpret_cast<int*>(&animator->currentAnimation), 0, animator->sprite ? (int)animator->sprite->animations.size() - 1 : 0);
							ImGui::SliderInt("Frame", reinterpret_cast<int*>(&animator->currentFrame), 0, animator->sprite ? (int)animator->sprite->animations[animator->currentAnimation].frames.size() - 1 : 0);
							ImGui::SliderInt("Layer", &animator->layer, -20, 20);

							if (ImGui::Button("Remove")) { entity->remove<Animator>(); }

							ImGui::TreePop();
						}
					}
					// --- Collider2D
					if (entity->has<Collider2D>())
					{
						Collider2D* collider = entity->get<Collider2D>();
						snprintf(buffer, 256, "Collider2D##%p", collider);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat2("Position", collider->position.data, 3);
							ImGui::InputFloat2("Size", collider->size.data, 3);
							ImGui::InputFloat("Bouncing", &collider->bouncing, 0.1f, 1.f, 2);
							ImGui::InputFloat("Friction", &collider->friction, 0.1f, 1.f, 2);

							if (ImGui::Button("Remove")) { entity->remove<Collider2D>(); }

							ImGui::TreePop();
						}
					}
					// --- RigidBody2D
					if (entity->has<RigidBody2D>())
					{
						RigidBody2D* rigid = entity->get<RigidBody2D>();
						snprintf(buffer, 256, "RigidBody2D##%p", rigid);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat("Mass", &rigid->mass, 0.1f, 1.f, 2);
							ImGui::InputFloat2("Acceleration", rigid->acceleration.data, 3);
							ImGui::InputFloat2("Velocity", rigid->velocity.data, 3);

							if (ImGui::Button("Remove")) { entity->remove<RigidBody2D>(); }

							ImGui::TreePop();
						}
					}
					// --- TileMap
					if (entity->has<TileMap>())
					{
						TileMap* map = entity->get<TileMap>();
						snprintf(buffer, 256, "TileMap##%p", map);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							vec2i gridCount = vec2i(map->gridCount);
							vec2i gridSize = vec2i(map->gridSize);
							if (ImGui::InputInt2("Grid count", gridCount.data))
								map->gridCount = vec2u(gridCount);
							if (ImGui::InputInt2("Grid size", gridSize.data))
								map->gridSize = vec2u(gridSize);
							if (map->texture == nullptr)
							{
								// TODO load texture button
							}
							else
							{
								float ratio = static_cast<float>(map->texture->width()) / static_cast<float>(map->texture->height());
								ImGui::Image((void*)map->texture->handle().value(), ImVec2(384, 384 * 1 / ratio), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), color);
							}
							if (ImGui::Button("Remove")) { entity->remove<TileMap>(); }

							ImGui::TreePop();
						}
					}
					// --- TileLayer
					if (entity->has<TileLayer>())
					{
						TileLayer* layer = entity->get<TileLayer>();
						snprintf(buffer, 256, "TileLayer##%p", layer);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							vec2i gridCount = vec2i(layer->gridCount);
							vec2i gridSize = vec2i(layer->gridSize);
							if (ImGui::InputInt2("Grid count", gridCount.data))
								layer->gridCount = vec2u(gridCount);
							if (ImGui::InputInt2("Grid size", gridSize.data))
								layer->gridSize = vec2u(gridSize);

							static int id = 0;
							if (layer->tileID.size() >= 4)
							{
								ImGui::SliderInt("Index", &id, 0, (int)layer->tileID.size() - 4);
								ImGui::InputInt4("TileID", layer->tileID.data() + id);
							}
							ImGui::SliderInt("Layer", &layer->layer, -20, 20);
							ImGui::InputFloat4("Color", layer->color.data, 3);

							if (ImGui::Button("Remove")) { entity->remove<TileLayer>(); }

							ImGui::TreePop();
						}
					}
					// --- Text
					if (entity->has<Text>())
					{
						Text* text = entity->get<Text>();
						snprintf(buffer, 256, "Text##%p", text);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat4("Color", text->color.data, 3);
							ImGui::SliderInt("Layer", &text->layer, -20, 20);

							uint32_t currentHeight = text->font->height();
							const char* currentFont = text->font->family().c_str();
							snprintf(buffer, 256, "%s (%u)", text->font->family().c_str(), text->font->height());
							if (ImGui::BeginCombo("Font", buffer))
							{
								for (auto& font : resources.font)
								{
									bool sameHeight = (currentHeight == font.second->height());
									bool sameFamily = (currentFont == font.second->family().c_str());
									bool sameFont = sameHeight && sameFamily;
									snprintf(buffer, 256, "%s (%u)", font.second->family().c_str(), font.second->height());
									if (ImGui::Selectable(buffer, sameFont))
									{
										if (!sameFont)
											text->font = font.second;
									}
									if (sameFont)
										ImGui::SetItemDefaultFocus();
								}
								ImGui::EndCombo();
							}

							char t[256];
							strcpy_s(t, text->text.c_str());
							if (ImGui::InputText("Text", t, 256))
								text->text = t;

							if (ImGui::Button("Remove")) { entity->remove<Text>(); }

							ImGui::TreePop();
						}
					}
					// --- Coin
					if (entity->has<Coin>())
					{
						Coin* coin = entity->get<Coin>();
						snprintf(buffer, 256, "Coin##%p", coin);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::Checkbox("Picked", &coin->picked);

							if (ImGui::Button("Remove")) { entity->remove<Coin>(); }

							ImGui::TreePop();
						}
					}
					// --- Player
					if (entity->has<Player>())
					{
						Player* player = entity->get<Player>();
						snprintf(buffer, 256, "Player##%p", player);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::SliderInt("Coin", &player->coin, 0, 50);
							float metric = player->speed.metric();
							if (ImGui::SliderFloat("Speed", &metric, 0.f, 50.f))
							{
								player->speed = Speed(metric);
							}
							player->state;
							player->jump;
							player->left;
							player->right;

							if (ImGui::Button("Remove")) { entity->remove<Player>(); }

							ImGui::TreePop();
						}
					}
					// --- Camera2D
					if (entity->has<Camera2D>())
					{
						Camera2D* camera = entity->get<Camera2D>();
						snprintf(buffer, 256, "Camera2D##%p", camera);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat2("Position", camera->position.data, 3);
							ImGui::InputFloat2("Viewport", camera->viewport.data, 3);

							if (ImGui::Button("Remove")) { entity->remove<Camera2D>(); }

							ImGui::TreePop();
						}
					}
					// --- SoundInstance
					if (entity->has<SoundInstance>())
					{
						SoundInstance* sound = entity->get<SoundInstance>();
						snprintf(buffer, 256, "SoundInstance##%p", sound);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::Text("Sound : %s", sound->path.c_str());
							ImGui::SliderFloat("Volume", &sound->volume, 0.f, 2.f);
							ImGui::Checkbox("Loop", &sound->loop);

							if (ImGui::Button("Remove")) { entity->remove<SoundInstance>(); }

							ImGui::TreePop();
						}
					}
					// --- Add component
					static int currentComponent = 0;
					if (ImGui::Button("Add")) {
						switch (currentComponent)
						{
						case 1: entity->add<Transform2D>(Transform2D()); break;
						case 2: entity->add<Animator>(Animator()); break;
						case 3: entity->add<Collider2D>(Collider2D()); break;
						case 4: entity->add<RigidBody2D>(RigidBody2D()); break;
						case 5: entity->add<Text>(Text()); break;
						case 6: entity->add<TileMap>(TileMap()); break;
						case 7: entity->add<TileLayer>(TileLayer()); break;
						case 8: entity->add<Coin>(Coin()); break;
						case 9: entity->add<Player>(Player()); break;
						case 10: entity->add<Camera2D>(Camera2D()); break;
						case 11: entity->add<SoundInstance>(SoundInstance()); break;
						}
					}
					ImGui::SameLine();
					ImGui::Combo("Component", &currentComponent, components, componentCount);
					ImGui::TreePop();
				}
				ImGui::Separator();
			});
			if (ImGui::Button("Add entity")) {
				world.createEntity();
			}
		}
		if (ImGui::CollapsingHeader("Resources##header", ImGuiTreeNodeFlags_DefaultOpen))
		{
		}
	}
	ImGui::End();
#endif
}*/

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
		m_gui.render();
	}

}

bool Game::running()
{
	return !input::pressed(input::Key::Escape);
}

}