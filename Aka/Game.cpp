#include "Game.h"

#include "Platform/Platform.h"
#include "Platform/Input.h"
#include "Platform/Logger.h"
#include "Platform/IO/FileSystem.h"
#include "Platform/Time.h"
#include "Component/Transform2D.h"
#include "Component/Collider2D.h"
#include "Component/Camera2D.h"
#include "Component/Text.h"
#include "Component/TileMap.h"
#include "Component/TileLayer.h"
#include "Component/Player.h"
#include "Component/Coin.h"
#include "Core/OgmoWorld.h"
#include "Core/ECS/World.h"
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
#include "Graphic/Renderer/GLRenderer.h"
#include "Graphic/Renderer/D3D11Renderer.h"

#define USE_IMGUI

#include <sstream>
#if defined(USE_IMGUI)
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#if defined(AKA_USE_OPENGL)
#include <examples/imgui_impl_opengl3.h>
#endif
#if defined(AKA_USE_D3D11)
#include <examples/imgui_impl_dx11.h>
#endif
#endif

namespace aka {

void Game::initialize(Window& window)
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
		m_fonts.push_back(std::make_shared<Font>(Asset::path("font/Espera/Espera-Bold.ttf"), 48));
		m_fonts.push_back(std::make_shared<Font>(Asset::path("font/Espera/Espera-Bold.ttf"), 16));
		m_fonts.push_back(std::make_shared<Font>(Asset::path("font/Theboldfont/theboldfont.ttf"), 48));
		m_textEntity = m_world.createEntity();
		Transform2D* transform = m_textEntity->add<Transform2D>(Transform2D());
		Text* text = m_textEntity->add<Text>(Text());

		text->offset = vec2f(0.f);
		text->color = color4f(1.f);
		text->font = m_fonts.back().get();
		text->text = "Find them all !";
		text->layer = 2;
		vec2i size = m_fonts.back()->size(text->text);
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
		m_sprites.push_back(std::make_shared<Sprite>());
		m_sprites.back()->animations.push_back(animation);

		m_backgroundEntity = m_world.createEntity();
		m_backgroundEntity->add<Animator>(Animator(m_sprites.back().get(), -2))->play("default");
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

		// - Colliders
		Sprite::Animation animation;
		animation.name = "default";
		Image image = Image::load(Asset::path("textures/debug/collider.png"));
		animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
		m_sprites.push_back(std::make_shared<Sprite>());
		m_sprites.back()->animations.push_back(animation);
		
		const OgmoLevel::Layer* layer = ogmoLevel.getLayer("Colliders");
		for (const OgmoLevel::Entity& entity : layer->entities)
		{
			Entity* collider = m_world.createEntity();
			collider->add<Transform2D>(Transform2D(vec2f((float)entity.position.x, (float)(layer->getHeight() - entity.position.y - entity.size.y)), vec2f(entity.size) / 16.f, radianf(0.f)));
			collider->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f)));
			//collider->add<Animator>(Animator(m_sprites.back().get(), 1));
		}
	}

	{
		// INIT CHARACTER
		// TODO move initialisation in SpriteAnimator & parse ase sprite directly ?
		// TODO load gif ?
		Sprite::Animation animation;
		animation.name = "idle";
		std::vector<Path> frames = {
			Asset::path("textures/player/player01.png"),
			Asset::path("textures/player/player02.png"),
			Asset::path("textures/player/player03.png")
		};
		uint32_t width = 0, height = 0;
		for (Path path : frames)
		{
			Image image = Image::load(path);
			width = image.width;
			height = image.height;
			animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
		}
		m_sprites.push_back(std::make_shared<Sprite>());
		m_sprites.back()->animations.push_back(animation);
		m_playerEntity = m_world.createEntity();
		m_playerEntity->add<Transform2D>(Transform2D(vec2f(80, 224), vec2f(1.f), radianf(0)));
		m_playerEntity->add<Animator>(Animator(m_sprites.back().get(), 1))->play("idle");
		m_playerEntity->add<RigidBody2D>(RigidBody2D(1.f));
		m_playerEntity->add<Collider2D>(Collider2D(vec2f(0.f), vec2f((float)width, (float)height), 0.1f, 0.1f));

		Player *player = m_playerEntity->add<Player>(Player());
		player->jump = Control(input::Key::Space);
		player->left = Control(input::Key::Q);
		player->right = Control(input::Key::D);

		m_playerEntity->add<Text>(Text(vec2f(3.f, 17.f), m_fonts[1].get(), "0", color4f(1.f), 3));
	}

	{
		// INIT COINS
		Sprite::Animation animation;
		animation.name = "picked";
		vec2f characterSize = vec2f(13, 17);
		std::vector<Path> frames = {
			Asset::path("textures/interact/coin01.png"),
			Asset::path("textures/interact/coin02.png"),
			Asset::path("textures/interact/coin03.png")
		};
		uint32_t width = 0, height = 0;
		for (Path path : frames)
		{
			Image image = Image::load(path);
			width = image.width;
			height = image.height;
			animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(100)));
		}
		m_sprites.push_back(std::make_shared<Sprite>());
		m_sprites.back()->animations.push_back(animation);
		Texture::Ptr tex = animation.frames[0].texture;
		animation.frames.clear();
		animation.name = "idle";
		animation.frames.push_back(Sprite::Frame::create(tex, Time::Unit::milliseconds(500)));
		m_sprites.back()->animations.push_back(animation);

		Entity *e = m_world.createEntity();
		e->add<Transform2D>(Transform2D(vec2f(80, 80), vec2f(1.f), radianf(0)));
		e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f), 0.1f, 0.1f));
		e->add<Animator>(Animator(m_sprites.back().get(), 1))->play("idle");
		e->add<Coin>(Coin());

		e = m_world.createEntity();
		e->add<Transform2D>(Transform2D(vec2f(100, 80), vec2f(1.f), radianf(0)));
		e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f), 0.1f, 0.1f));
		e->add<Animator>(Animator(m_sprites.back().get(), 1))->play("idle");
		e->add<Coin>(Coin());

		e = m_world.createEntity();
		e->add<Transform2D>(Transform2D(vec2f(120, 80), vec2f(1.f), radianf(0)));
		e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f), 0.1f, 0.1f));
		e->add<Animator>(Animator(m_sprites.back().get(), 1))->play("idle");
		e->add<Coin>(Coin());

		e = m_world.createEntity();
		e->add<Transform2D>(Transform2D(vec2f(260, 96), vec2f(1.f), radianf(0)));
		e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f), 0.1f, 0.1f));
		e->add<Animator>(Animator(m_sprites.back().get(), 1))->play("idle");
		e->add<Coin>(Coin());

		e = m_world.createEntity();
		e->add<Transform2D>(Transform2D(vec2f(280, 96), vec2f(1.f), radianf(0)));
		e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f), 0.1f, 0.1f));
		e->add<Animator>(Animator(m_sprites.back().get(), 1))->play("idle");
		e->add<Coin>(Coin());
	}

	{
		window.setSizeLimits(m_framebuffer->width(), m_framebuffer->height(), GLFW_DONT_CARE, GLFW_DONT_CARE);
	}
#if defined(USE_IMGUI)
	{
		// IMGUI
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

#if defined(AKA_USE_OPENGL)
		ImGui_ImplGlfw_InitForOpenGL(window.handle(), true);

		float glLanguageVersion = (float)atof((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		std::stringstream ss;
		ss << "#version " << (GLuint)(100.f * glLanguageVersion) << std::endl;
		ImGui_ImplOpenGL3_Init(ss.str().c_str());
#else
		D3D11Renderer* renderer = reinterpret_cast<D3D11Renderer*>(GraphicBackend::renderer());
		ImGui_ImplDX11_Init(renderer->context().device, renderer->context().deviceContext);
#endif
		ImGui::StyleColorsDark();
	}
	{
		// Initialize everything
		m_world.create();
	}
#endif
}

void Game::destroy()
{
#if defined(USE_IMGUI)
#if defined(AKA_USE_OPENGL)
	ImGui_ImplOpenGL3_Shutdown();
#else
	ImGui_ImplDX11_Shutdown();
#endif
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
#endif
}

void Game::frame()
{
#if defined(USE_IMGUI)
	// Start the Dear ImGui frame
#if defined(AKA_USE_OPENGL)
	ImGui_ImplOpenGL3_NewFrame();
#else
	ImGui_ImplDX11_NewFrame();
#endif
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
#endif
}
// Need a component movable -> take inputs and update movements
// Component hurtable
// collision -> if hurtable and collider has component damage
void Game::update(Time::Unit deltaTime)
{
	if (input::pressed(input::Key::F1))
	{
#if defined(AKA_USE_OPENGL)
		glFinish();
		Image image;
		image.width = screenWidth();
		image.height = screenHeight();
		image.bytes.resize(image.width * image.height * 4);
		std::vector<uint8_t> bytes(image.bytes.size());
		glReadPixels(0, 0, screenWidth(), screenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, bytes.data());
		uint32_t stride = 4 * image.width;
		for (uint32_t y = 0; y < image.height; y++)
			memcpy(image.bytes.data() + stride * y, bytes.data() + image.bytes.size() - stride - stride * y, stride);
		image.save("./output.jpg");
#endif
	}
	if (input::down(input::Key::H))
	{
		m_displayUI = !m_displayUI;
	}
	// Update world after moving manually objects
	m_world.update(deltaTime);
}

void Game::renderGUI()
{
#if defined(USE_IMGUI)
	if (ImGui::Begin("Editor##window"))
	{
		ImVec4 color = ImVec4(236.f / 255.f, 11.f / 255.f, 67.f / 255.f, 1.f);
		if (ImGui::CollapsingHeader("Infos", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("Resolution : %ux%u", screenWidth(), screenHeight());
			ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
			ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);

			static bool renderColliders = false;
			if (ImGui::Checkbox("Render colliders", &renderColliders))
			{
				m_world.each<Collider2D>([&](Entity * entity, Collider2D * collider) {
					if (renderColliders)
					{
						if(!entity->has<Animator>())
							entity->add<Animator>(Animator(m_sprites[1].get(), 2));
					}
					else
					{
						if (entity->has<Animator>() && !entity->has<Player>() && !entity->has<Coin>())
							entity->remove<Animator>();
					}
				});
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
			m_world.each([&](Entity* entity) {
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
									Sprite::Animation animation;
									animation.name = "default";
									Image image = Image::load(Asset::path(path));
									animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
									m_sprites.push_back(std::make_shared<Sprite>());
									m_sprites.back()->animations.push_back(animation);
									animator->sprite = m_sprites.back().get();
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
								for (int n = 0; n < m_fonts.size(); n++)
								{
									bool sameHeight = (currentHeight == m_fonts[n]->height());
									bool sameFamily = (currentFont == m_fonts[n]->family().c_str());
									bool sameFont = sameHeight && sameFamily;
									snprintf(buffer, 256, "%s (%u)", m_fonts[n]->family().c_str(), m_fonts[n]->height());
									if (ImGui::Selectable(buffer, sameFont))
									{
										if (!sameFont)
											text->font = m_fonts[n].get();
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
				m_world.createEntity();
			}
		}
		if (ImGui::CollapsingHeader("Resources##header", ImGuiTreeNodeFlags_DefaultOpen))
		{
		}
	}
	ImGui::End();
#endif
}

void Game::render()
{
	//backend.viewport(0, 0, framebuffer->width(), framebuffer->height());
	m_framebuffer->bind(Framebuffer::Type::Both);
	m_framebuffer->clear(1.f, 0.63f, 0.f, 1.f);

	// Render
	Camera2D *camera = m_cameraEntity->get<Camera2D>();
	mat4f view = mat4f::inverse(mat4f(
		col4f(1.f, 0.f, 0.f, 0.f),
		col4f(0.f, 1.f, 0.f, 0.f),
		col4f(0.f, 0.f, 1.f, 0.f),
		col4f(camera->position.x, camera->position.y, 0.f, 1.f)
	));
	mat4f projection = mat4f::orthographic(0.f, static_cast<float>(m_framebuffer->height()), 0.f, static_cast<float>(m_framebuffer->width()), -1.f, 1.f);
	m_world.draw(m_batch);
	m_batch.render(m_framebuffer, view, projection);

	// Blit to main buffer
	uint32_t widthRatio = screenWidth() / m_framebuffer->width();
	uint32_t heightRatio = screenHeight() / m_framebuffer->height();
	uint32_t ratio = min<uint32_t>(widthRatio, heightRatio);
	uint32_t scaledWidth = ratio * m_framebuffer->width();
	uint32_t scaledHeight = ratio * m_framebuffer->height();
	uint32_t w = (screenWidth() - scaledWidth) / 2;
	uint32_t h = (screenHeight() - scaledHeight) / 2;
	Rect srcBlit{};
	srcBlit.x = 0.f;
	srcBlit.y = 0.f;
	srcBlit.w = (float)m_framebuffer->width();
	srcBlit.h = (float)m_framebuffer->height();
	Rect dstBlit{};
	dstBlit.x = (float)w;
	dstBlit.y = (float)h;
	dstBlit.w = (float)screenWidth() - 2.f * w;
	dstBlit.h = (float)screenHeight() - 2.f * h;

	GraphicBackend::backbuffer()->bind(Framebuffer::Type::Both);
	GraphicBackend::viewport(0, 0, screenWidth(), screenHeight());
	GraphicBackend::backbuffer()->clear(0.f, 0.f, 0.f, 1.f);
	m_framebuffer->blit(GraphicBackend::backbuffer(), srcBlit, dstBlit, Sampler::Filter::Nearest);

	// Rendering imgui
	if (m_displayUI)
		renderGUI();

#if defined(USE_IMGUI)
	ImGui::Render();
#if defined(AKA_USE_OPENGL)
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#else
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
#endif
}

}