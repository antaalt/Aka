#include "Game.h"

#include "Camera2D.h"
#include "Platform.h"
#include "Image.h"
#include "Input.h"
#include "Time.h"
#include "Logger.h"
#include "FileSystem.h"
#include "Batch.h"

#include "Text.h"
#include "World.h"
#include "PhysicSystem.h"
#include "Transform2D.h"
#include "Collider2D.h"
#include "AnimatorSystem.h"
#include "TileMapSystem.h"
#include "TileRenderSystem.h"
#include "TextRenderSystem.h"
#include "CameraSystem.h"

#include "OgmoWorld.h"

#include <sstream>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

namespace aka {

Font font;
Sprite character;
Sprite background;
Sprite colliderSprite;
Framebuffer::Ptr framebuffer;



Time::Unit lastTick;
const vec2u viewportSize(320, 180);

// Entity
Entity* characterEntity;
Entity* backgroundEntity;
Entity* textEntity;
// Systems
PhysicSystem *physicSystem;
AnimatorSystem *animatorSystem;
TileMapSystem* tileMapSystem;
TileSystem* tileSystem;
CameraSystem* cameraSystem;
TextRenderSystem* textRenderingSystem;


void Game::initialize(Window& window, GraphicBackend& backend)
{
	lastTick = Time::now();
	Logger::debug.mute();
	physicSystem = m_world.createSystem<PhysicSystem>(Time::Unit(10));
	tileSystem = m_world.createSystem<TileSystem>();
	tileMapSystem = m_world.createSystem<TileMapSystem>();
	animatorSystem = m_world.createSystem<AnimatorSystem>();
	cameraSystem = m_world.createSystem<CameraSystem>();
	textRenderingSystem = m_world.createSystem<TextRenderSystem>();

	{
		// INIT FIXED TEXTURE BACKGROUND
		Image image = Image::load(Asset::path("textures/background/background.png"));
		ASSERT(image.width == viewportSize.x, "incorrect width");
		ASSERT(image.height == viewportSize.y, "incorrect height");

		Sprite::Animation animation;
		animation.name = "default";
		animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba8, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), 500));
		background.animations.push_back(animation);

		backgroundEntity = m_world.createEntity();
		backgroundEntity->add<Transform2D>(Transform2D(vec2f(0), vec2f(viewportSize), radianf(0)));
		backgroundEntity->add<Animator>(Animator(&background, -2))->play("default");
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
		animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba8, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), 500));
		colliderSprite.animations.push_back(animation);
		
		const OgmoLevel::Layer* layer = ogmoLevel.getLayer("Colliders");
		for (const OgmoLevel::Entity& entity : layer->entities)
		{
			Entity* collider = m_world.createEntity();
			collider->add<Transform2D>(Transform2D(vec2f((float)entity.position.x, (float)(layer->getHeight() - entity.position.y - entity.size.y)), vec2f(entity.size), radianf(0.f)));
			collider->add<Collider2D>(Collider2D());
			collider->add<Animator>(Animator(&colliderSprite, 1));
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
			animation.frames.push_back(Sprite::Frame::create(Texture::create(image.width, image.height, Texture::Format::Rgba8, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest), 500));
		}
		character.animations.push_back(animation);
		characterEntity = m_world.createEntity();
		characterEntity->add<Transform2D>(Transform2D(vec2f(viewportSize) / 2.f, vec2f((float)width, (float)height), radianf(0)));
		characterEntity->add<Animator>(Animator(&character, 1))->play("idle");
		characterEntity->add<RigidBody2D>(RigidBody2D(1.f, 0.1f, 0.1f));
		characterEntity->add<Collider2D>(Collider2D());
	}

	{
		// INIT FONTS
		font = Font::create(Asset::path("font/Espera/Espera-Bold.ttf"), 48);
		textEntity = m_world.createEntity();
		Transform2D* transform = textEntity->add<Transform2D>(Transform2D());
		Text* text = textEntity->add<Text>(Text());

		text->color = color4f(1.f);
		text->font = &font;
		text->text = "Hello World !";
		text->layer = 3;
		vec2i size = font.size(text->text);
		transform->position = vec2f((float)((int)viewportSize.x / 2 - size.x / 2), (float)((int)viewportSize.y / 2 - size.y / 2));
		transform->size = vec2f(1.f);
	}

	// INIT FRAMEBUFFER
	framebuffer = Framebuffer::create(viewportSize.x, viewportSize.y, Sampler::Filter::Nearest);

	window.setSizeLimits((int32_t)viewportSize.x, (int32_t)viewportSize.y, GLFW_DONT_CARE, GLFW_DONT_CARE);
	
	// INIT CHARACTER

	// IMGUI
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window.getHandle()), true);

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

void Game::update(GraphicBackend& backend)
{
	Time::Unit now = Time::now();
	Time::Unit deltaTime = now - lastTick;
	lastTick = now;
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	Transform2D* transform = characterEntity->get<Transform2D>();
	RigidBody2D* rigid = characterEntity->get<RigidBody2D>();
	Collider2D* collider = characterEntity->get<Collider2D>();
	transform->position.x += input::pressed(input::Key::D) - input::pressed(input::Key::Q);
	transform->position.y += input::pressed(input::Key::Z) - input::pressed(input::Key::S);

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
		transform->position = vec2f(transform->position.x, (float)viewportSize.y);
	}

	if (ImGui::Begin("Debug##window"))
	{
		ImVec4 color= ImVec4(236.f / 255.f, 11.f / 255.f, 67.f / 255.f, 1.f);
		if (ImGui::CollapsingHeader("Infos", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGuiIO &io = ImGui::GetIO();
			ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
			ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		}

		if (ImGui::CollapsingHeader("Colliders##header", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static bool renderColliders = true;
			if (ImGui::Checkbox("Render##checkbox", &renderColliders))
			{
				if (renderColliders)
				{
					m_world.each<Collider2D>([&](Entity* entity, Collider2D* collider) {
						entity->add<Animator>(Animator(&colliderSprite, 1));
					});
				}
				else
				{
					m_world.each<Collider2D>([&](Entity* entity, Collider2D* collider) {
						if (entity->get<Animator>()->sprite == &colliderSprite)
							entity->remove<Animator>();
					});
				}
			}
			char buffer[256];
			ImGui::TextColored(color, "Dynamics");
			m_world.each<Transform2D, RigidBody2D, Collider2D>([&](Entity* entity, Transform2D* transform, RigidBody2D* rigid, Collider2D* collider) {
				snprintf(buffer, 256, "RigidBody##%p", entity);
				if (ImGui::TreeNode(buffer))
				{
					/*switch (type)
					{
					case Shape2D::Type::Rect: {
						Rect* rect = reinterpret_cast<Rect*>(dynamic->getShape());
						ImGui::InputFloat2("Position", rect->position.data, 3);
						ImGui::InputFloat2("Size", rect->size.data, 3);
						break;
					}
					default:
						break;
					}*/
					ImGui::InputFloat2("Position", transform->position.data, 3);
					ImGui::InputFloat2("Size", transform->size.data, 3);

					ImGui::InputFloat("Mass", &rigid->mass, 0.1f, 1.f, 2);
					ImGui::InputFloat("Bouncing", &rigid->bouncing, 0.1f, 1.f, 2);
					ImGui::InputFloat("Friction", &rigid->friction, 0.1f, 1.f, 2);
					ImGui::InputFloat2("Acceleration", rigid->acceleration.data, 3);
					ImGui::InputFloat2("Velocity", rigid->velocity.data, 3);
					ImGui::TreePop();
				}
			});
			static int current_shape_dyn = 0;
			const char* current_shape_dyn_name[2] = {
				"Rect",
				"Circle"
			};
			if (ImGui::Button("Add##dynamic"))
			{
				if (current_shape_dyn == 0)
				{
					Entity* e = m_world.createEntity();
					e->add<Transform2D>(Transform2D(vec2f(0.f), vec2f(16.f), radianf(0.f)));
					e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f)));
					e->add<RigidBody2D>(RigidBody2D(1.f));
				}
				else
				{

				}
			}
			ImGui::SameLine();
			ImGui::Combo("Shape##dynamic", &current_shape_dyn, current_shape_dyn_name, 2);
			ImGui::Separator();


			ImGui::TextColored(color, "Statics");
			m_world.each<Transform2D, Collider2D>([&](Entity* entity, Transform2D* transform, Collider2D* collider) {
				snprintf(buffer, 256, "Collider##%p", collider);
				if (ImGui::TreeNode(buffer))
				{
					/*switch (type)
					{
					case Shape2D::Type::Rect: {
						Rect* rect = reinterpret_cast<Rect*>(staticc->getShape());
						ImGui::InputFloat2("Position", rect->position.data, 3);
						ImGui::InputFloat2("Size", rect->size.data, 3);
						break;
					}
					default:
						break;
					}*/
					ImGui::InputFloat2("Position", transform->position.data, 3);
					ImGui::InputFloat2("Size", transform->size.data, 3);
					ImGui::TreePop();
				}
			});
			static int current_shape = 0;
			const char* current_shape_name[2] = {
				"Rect",
				"Circle"
			};
			if (ImGui::Button("Add"))
			{
				if (current_shape == 0)
				{
					Entity* e = m_world.createEntity();
					e->add<Transform2D>(Transform2D(vec2f(0.f), vec2f(16.f), radianf(0.f)));
					e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f)));
				}
				else
				{

				}
			}
			ImGui::SameLine();
			ImGui::Combo("Shape##static", &current_shape, current_shape_name, 2);
			
		}
		if (ImGui::CollapsingHeader("Level##header", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextColored(color, "Layers");
			static const char* layerTypeName[] = {
				"Tile",
				"Grid",
				"Entity"
			};
			m_world.each<TileLayer, TileMap>([&](Entity* entity, TileLayer* layer, TileMap* map) {
				char buffer[256];
				snprintf(buffer, 256, "Layer##%p", layer);
				if (ImGui::TreeNode(buffer))
				{
					vec2i gridCount = vec2i(layer->gridCount);
					vec2i gridSize = vec2i(layer->gridSize);
					if (ImGui::InputInt2("Grid count", gridCount.data))
						layer->gridCount = vec2u(gridCount);
					if (ImGui::InputInt2("Grid size", gridSize.data))
						layer->gridSize = vec2u(gridSize);
					ImGui::Image((void*)(uintptr_t)(map->texture->id()), ImVec2(512, 512), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 0, 0, 1));

					static int id = 0;
					ImGui::SliderInt("", &id, 0, (int)layer->tileID.size() - 4);
					if (ImGui::InputInt4("", layer->tileID.data() + id))
					{
						// TODO update TBO
						//layer->tbo;
					}

					ImGui::TreePop();
				}
			});
		}
	}
	ImGui::End();

	// Update world after moving manually objects
	m_world.update(deltaTime);
}

void Game::render(GraphicBackend& backend)
{
	Batch batch;
	backend.viewport(0, 0, (uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
	framebuffer->bind(Framebuffer::Type::Both);
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	// draw background
	m_world.draw(batch);
	batch.render(framebuffer);

	// Blit to main buffer
	uint32_t widthRatio = screenWidth() / (uint32_t)viewportSize.x;
	uint32_t heightRatio = screenHeight() / (uint32_t)viewportSize.y;
	uint32_t ratio = min<uint32_t>(widthRatio, heightRatio);
	uint32_t scaledWidth = ratio * (uint32_t)viewportSize.x;
	uint32_t scaledHeight = ratio * (uint32_t)viewportSize.y;
	uint32_t w = (screenWidth() - scaledWidth) / 2;
	uint32_t h = (screenHeight() - scaledHeight) / 2;
	Rect srcBlit{};
	srcBlit.x = 0.f;
	srcBlit.y = 0.f;
	srcBlit.w = (float)viewportSize.x;
	srcBlit.h = (float)viewportSize.y;
	Rect dstBlit{};
	dstBlit.x = (float)w;
	dstBlit.y = (float)h;
	dstBlit.w = (float)screenWidth() - 2.f * w;
	dstBlit.h = (float)screenHeight() - 2.f * h;

	backend.backbuffer()->bind(Framebuffer::Type::Both);
	backend.viewport(0, 0, screenWidth(), screenHeight());
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	framebuffer->blit(backend.backbuffer(), srcBlit, dstBlit, Sampler::Filter::Nearest);

	// Rendering imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}