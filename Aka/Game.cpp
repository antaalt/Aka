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
#include "Core/OgmoWorld.h"
#include "Core/World.h"
#include "System/PhysicSystem.h"
#include "System/AnimatorSystem.h"
#include "System/TileMapSystem.h"
#include "System/TileRenderSystem.h"
#include "System/TextRenderSystem.h"
#include "System/CameraSystem.h"
#include "System/CollisionSystem.h"

#include <sstream>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

namespace aka {


void Game::initialize(Window& window, GraphicBackend& backend)
{
	Logger::debug.mute();
	m_world.createSystem<PhysicSystem>();
	m_world.createSystem<CollisionSystem>();
	m_world.createSystem<TileSystem>();
	m_world.createSystem<TileMapSystem>();
	m_world.createSystem<AnimatorSystem>();
	m_world.createSystem<CameraSystem>();
	m_world.createSystem<TextRenderSystem>();

	// INIT FRAMEBUFFER
	m_framebuffer = Framebuffer::create(320, 180, Sampler::Filter::Nearest);

	{
		// INIT FIXED TEXTURE BACKGROUND
		Image image = Image::load(Asset::path("textures/background/background.png"));
		ASSERT(image.width == m_framebuffer->width(), "incorrect width");
		ASSERT(image.height == m_framebuffer->height(), "incorrect height");

		Sprite::Animation animation;
		animation.name = "default";
		animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba8, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
		m_sprites.push_back(std::make_shared<Sprite>());
		m_sprites.back()->animations.push_back(animation);

		m_backgroundEntity = m_world.createEntity();
		m_backgroundEntity->add<Transform2D>(Transform2D(vec2f(0), vec2f((float)m_framebuffer->width(), (float)m_framebuffer->height()), radianf(0)));
		m_backgroundEntity->add<Animator>(Animator(m_sprites.back().get(), -2))->play("default");
	}
	{
		// INIT world
		OgmoWorld ogmoWorld = OgmoWorld::load(Asset::path("levels/world.ogmo"));
		OgmoLevel ogmoLevel = OgmoLevel::load(ogmoWorld, Asset::path("levels/level0.json"));
		const OgmoLevel::Layer* foreground = ogmoLevel.getLayer("Foreground");

		// Layers
		auto createTileLayer = [&](const OgmoLevel::Layer* layer, int32_t layerDepth) -> Entity* {
			if (layer->layer->type != OgmoWorld::LayerType::Tile)
				return nullptr;
			ASSERT(layer->tileset->tileSize == layer->gridCellSize, "");
			Texture::Ptr texture = Texture::create(layer->tileset->image.width, layer->tileset->image.height, Texture::Format::Rgba8, Texture::Format::Rgba, layer->tileset->image.bytes.data(), Sampler::Filter::Nearest);
			
			Entity* entity = m_world.createEntity();
			Transform2D* transform = entity->add<Transform2D>(Transform2D(vec2f(0.f), vec2f(vec2u(layer->getWidth(), layer->getHeight())), radianf(0.f)));
			TileMap* tileMap = entity->add<TileMap>(TileMap(layer->tileset->tileCount, layer->tileset->tileSize, texture));
			TileLayer* tileLayer = entity->add<TileLayer>(TileLayer(layer->gridCellCount, layer->gridCellSize, color4f(1.f), layer->data, layerDepth));
			return entity;
		};
		createTileLayer(ogmoLevel.getLayer("Background"), -1);
		createTileLayer(ogmoLevel.getLayer("Playerground"), 0);
		createTileLayer(ogmoLevel.getLayer("Foreground"), 1);
		// Colliders
		Sprite::Animation animation;
		animation.name = "default";
		Image image = Image::load(Asset::path("textures/debug/collider.png"));
		animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba8, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
		m_sprites.push_back(std::make_shared<Sprite>());
		m_sprites.back()->animations.push_back(animation);
		
		const OgmoLevel::Layer* layer = ogmoLevel.getLayer("Colliders");
		for (const OgmoLevel::Entity& entity : layer->entities)
		{
			Entity* collider = m_world.createEntity();
			collider->add<Transform2D>(Transform2D(vec2f((float)entity.position.x, (float)(layer->getHeight() - entity.position.y - entity.size.y)), vec2f(entity.size), radianf(0.f)));
			collider->add<Collider2D>(Collider2D());
			collider->add<Animator>(Animator(m_sprites.back().get(), 1));
		}
	}
	{
		// INIT SPRITE CHARACTER
		// TODO move initialisation in SpriteAnimator & parse ase sprite directly ?
		// TODO load gif ?
		Sprite::Animation animation;
		animation.name = "idle";
		vec2f characterSize = vec2f(13, 17);
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
			animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba8, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
		}
		m_sprites.push_back(std::make_shared<Sprite>());
		m_sprites.back()->animations.push_back(animation);
		m_characterEntity = m_world.createEntity();
		m_characterEntity->add<Transform2D>(Transform2D(vec2f((float)m_framebuffer->width(), (float)m_framebuffer->height()) / 2.f, vec2f((float)width, (float)height), radianf(0)));
		m_characterEntity->add<Animator>(Animator(m_sprites.back().get(), 1))->play("idle");
		m_characterEntity->add<RigidBody2D>(RigidBody2D(1.f, 0.1f, 0.1f));
		m_characterEntity->add<Collider2D>(Collider2D());
	}

	{
		// INIT FONTS
		m_fonts.push_back(std::make_shared<Font>(Asset::path("font/Espera/Espera-Bold.ttf"), 48));
		m_textEntity = m_world.createEntity();
		Transform2D* transform = m_textEntity->add<Transform2D>(Transform2D());
		Text* text = m_textEntity->add<Text>(Text());

		text->color = color4f(1.f);
		text->font = m_fonts.back().get();
		text->text = "Hello World !";
		text->layer = 3;
		vec2i size = m_fonts.back()->size(text->text);
		transform->position = vec2f((float)((int)m_framebuffer->width() / 2 - size.x / 2), (float)((int)m_framebuffer->height() / 2 - size.y / 2));
		transform->size = vec2f(1.f);
	}

	window.setSizeLimits(m_framebuffer->width(), m_framebuffer->height(), GLFW_DONT_CARE, GLFW_DONT_CARE);

	// IMGUI
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	ImGui_ImplGlfw_InitForOpenGL(window.handle(), true);

	float glLanguageVersion = (float)atof((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	std::stringstream ss;
	ss << "#version " << (GLuint)(100.f * glLanguageVersion) << std::endl;
	ImGui_ImplOpenGL3_Init(ss.str().c_str());
	ImGui::StyleColorsDark();

	// Initialize everything
	m_world.create();
}

void Game::destroy(GraphicBackend& backend)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Game::frame()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Game::update(Time::Unit deltaTime)
{
	Transform2D* transform = m_characterEntity->get<Transform2D>();
	RigidBody2D* rigid = m_characterEntity->get<RigidBody2D>();
	Collider2D* collider = m_characterEntity->get<Collider2D>();
	transform->position.x += (input::pressed(input::Key::D) - input::pressed(input::Key::Q)) * 64.f * deltaTime.seconds();
	transform->position.y += (input::pressed(input::Key::Z) - input::pressed(input::Key::S)) * 64.f * deltaTime.seconds();

	if (input::pressed(input::Key::Space))
	{
		rigid->acceleration = vec2f(0.f, 0.f);
		rigid->velocity = vec2f(0.f, 8.f); // 16 pixels / s
	}
	if (input::pressed(input::Key::LeftCtrl))
	{
		transform->position = vec2f(192.f, 160.f);
	}

	if (transform->position.y < -transform->size.y)
	{
		// teleport above
		transform->position = vec2f(transform->position.x, (float)m_framebuffer->height());
	}

	// Update world after moving manually objects
	m_world.update(deltaTime);
}

void Game::render(GraphicBackend& backend)
{
	//backend.viewport(0, 0, framebuffer->width(), framebuffer->height());
	m_framebuffer->bind(Framebuffer::Type::Both);
	backend.clear(0.f, 0.f, 0.f, 1.f);
	// draw background
	m_world.draw(m_batch);
	m_batch.render(m_framebuffer);

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

	backend.backbuffer()->bind(Framebuffer::Type::Both);
	backend.viewport(0, 0, screenWidth(), screenHeight());
	backend.clear(0.f, 0.f, 0.f, 1.f);
	m_framebuffer->blit(backend.backbuffer(), srcBlit, dstBlit, Sampler::Filter::Nearest);

	// Rendering imgui
	if (ImGui::Begin("Editor##window"))
	{
		ImVec4 color = ImVec4(236.f / 255.f, 11.f / 255.f, 67.f / 255.f, 1.f);
		if (ImGui::CollapsingHeader("Infos", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("Resolution : %ux%u", screenWidth(), screenHeight());
			ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
			ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		}

		if (ImGui::CollapsingHeader("Entities##header", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextColored(color, "Entities");
			// TODO add filter
			int index = 0;
			m_world.each([&](Entity* entity) {
				char buffer[256];
				snprintf(buffer, 256, "Entity %d", index++);
				if (ImGui::TreeNode(buffer))
				{
					// --- Transform2D
					if (entity->has<Transform2D>())
					{
						Transform2D* transform = entity->get<Transform2D>();
						snprintf(buffer, 256, "Transform2D##%p", transform);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat2("Position", transform->position.data, 3);
							ImGui::InputFloat2("Size", transform->size.data, 3);
							ImGui::InputFloat("Rotation", &transform->rotation(), 0.1f, 1.f, 3);

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
									animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba8, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), Time::Unit::milliseconds(500)));
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
												ImGui::Image((void*)(uintptr_t)(frame.texture->id().value()), ImVec2(256, 256 * 1 / ratio), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), color);

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
							ImGui::InputFloat("Bouncing", &rigid->bouncing, 0.1f, 1.f, 2);
							ImGui::InputFloat("Friction", &rigid->friction, 0.1f, 1.f, 2);
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
								ImGui::Image((void*)(uintptr_t)(map->texture->id().value()), ImVec2(384, 384 * 1 / ratio), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), color);
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

							static int currentFontID = 0;
							const char* currentFont = m_fonts[currentFontID]->family().c_str();
							if (ImGui::BeginCombo("Font", currentFont))
							{
								for (int n = 0; n < m_fonts.size(); n++)
								{
									bool is_selected = (currentFont == m_fonts[n]->family().c_str());
									if (ImGui::Selectable(m_fonts[n]->family().c_str(), is_selected))
									{
										if (n != currentFontID)
										{
											text->font = m_fonts[n].get();
											currentFontID = n;
										}
									}
									if (is_selected)
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
					// --- Add component
					static const int componentCount = 7;
					static const char* components[componentCount] = {
						"Transform2D",
						"Animator",
						"Collider2D",
						"RigidBody2D",
						"Text",
						"TileMap",
						"TileLayer",
					};
					static int currentComponent = 0;
					if (ImGui::Button("Add")) {
						switch (currentComponent)
						{
						case 0: entity->add<Transform2D>(Transform2D()); break;
						case 1:  entity->add<Animator>(Animator()); break;
						case 2: entity->add<Collider2D>(Collider2D()); break;
						case 3: entity->add<RigidBody2D>(RigidBody2D()); break;
						case 4: entity->add<Text>(Text()); break;
						case 5: entity->add<TileMap>(TileMap()); break;
						case 6: entity->add<TileLayer>(TileLayer()); break;
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

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}