#include "Game.h"

#include "Camera2D.h"
#include "Platform.h"
#include "Image.h"
#include "Input.h"
#include "Time.h"
#include "Logger.h"
#include "FileSystem.h"

#include "World.h"
#include "PhysicSystem.h"
#include "Transform2D.h"
#include "Collider2D.h"
#include "AnimatorSystem.h"
#include "TileMapSystem.h"
#include "TileRenderSystem.h"
#include "CameraSystem.h"

#include "OgmoWorld.h"

#include <sstream>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

namespace aka {

GLuint framebufferID;
Sprite character;
Sprite background;
Sprite colliderSprite;
Texture* renderTarget;
FontRenderer* fontRenderer;
Font font24, font48, font96;

const vec2u viewportSize(320, 180);
GLenum error = GL_NO_ERROR;

Entity* characterEntity;
Entity* backgroundEntity;
PhysicSystem *physicSystem;
AnimatorSystem *animatorSystem;
TileMapSystem* tileMapSystem;
TileSystem* tileSystem;
CameraSystem* cameraSystem;


void Game::initialize(Window& window, GraphicBackend& backend)
{
	physicSystem = m_world.createSystem<PhysicSystem>(Time::Unit(10));
	tileSystem = m_world.createSystem<TileSystem>();
	tileMapSystem = m_world.createSystem<TileMapSystem>();
	animatorSystem = m_world.createSystem<AnimatorSystem>();
	cameraSystem = m_world.createSystem<CameraSystem>();

	{
		// INIT FIXED TEXTURE BACKGROUND
		Image image = Image::load(Asset::path("textures/background/background.png"));
		ASSERT(image.width == viewportSize.x, "incorrect width");
		ASSERT(image.height == viewportSize.y, "incorrect height");

		Sprite::Animation animation;
		animation.name = "default";
		animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		background.animations.push_back(animation);

		backgroundEntity = m_world.createEntity();
		backgroundEntity->add<Transform2D>(Transform2D(vec2f(0), vec2f(viewportSize), radianf(0)));
		backgroundEntity->add<Animator>(Animator(&background, 0.f))->play("default");
	}
	{
		// INIT world
		OgmoWorld ogmoWorld = OgmoWorld::load(Asset::path("levels/world.ogmo"));
		OgmoLevel ogmoLevel = OgmoLevel::load(ogmoWorld, Asset::path("levels/level0.json"));
		const OgmoLevel::Layer* foreground = ogmoLevel.getLayer("Foreground");

		// Layers
		auto createTileLayer = [&](const OgmoLevel::Layer* layer, float depth) -> Entity* {
			if (layer->layer->type != OgmoWorld::LayerType::Tile)
				return nullptr;
			ASSERT(layer->tileset->tileSize == layer->gridCellSize, "");
			Texture* texture = backend.createTexture(layer->tileset->image.width, layer->tileset->image.height, layer->tileset->image.bytes.data());
			
			Entity* entity = m_world.createEntity();
			Transform2D* transform = entity->add<Transform2D>(Transform2D(vec2f(0.f), vec2f(vec2u(layer->getWidth(), layer->getHeight())), radianf(0.f)));
			//TileMap* tileMap = entity->add<TileMap>(TileMap(layer->tileset->tileCount, layer->tileset->tileSize, texture));
			//TileLayer* tileLayer = entity->add<TileLayer>(TileLayer(layer->gridCellCount, layer->gridCellSize, color4f(1.f), layer->data, depth));
			return entity;
		};
		createTileLayer(ogmoLevel.getLayer("Background"), 0.f);
		createTileLayer(ogmoLevel.getLayer("Playerground"), 0.f);
		createTileLayer(ogmoLevel.getLayer("Foreground"), 0.f);
		// Colliders
		Sprite::Animation animation;
		animation.name = "default";
		Image image = Image::load(Asset::path("textures/debug/collider.png"));
		animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		colliderSprite.animations.push_back(animation);
		
		const OgmoLevel::Layer* layer = ogmoLevel.getLayer("Colliders");
		for (const OgmoLevel::Entity& entity : layer->entities)
		{
			Entity* collider = m_world.createEntity();
			collider->add<Transform2D>(Transform2D(vec2f((float)entity.position.x, (float)(layer->getHeight() - entity.position.y - entity.size.y)), vec2f(entity.size), radianf(0.f)));
			collider->add<Collider2D>(Collider2D());
			collider->add<Animator>(Animator(&colliderSprite, 0.f));
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
		uint32_t width, height;
		for (Path path : frames)
		{
			Image image = Image::load(path);
			width = image.width;
			height = image.height;
			animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		}
		character.animations.push_back(animation);
		characterEntity = m_world.createEntity();
		characterEntity->add<Transform2D>(Transform2D(vec2f(viewportSize) / 2.f, vec2f(width, height), radianf(0)));
		characterEntity->add<Animator>(Animator(&character, 0.f))->play("idle");
		characterEntity->add<RigidBody2D>(RigidBody2D(1.f, 0.1f, 0.1f));
		characterEntity->add<Collider2D>(Collider2D());
	}

	// INIT FRAMEBUFFER RENDER TARGET
	renderTarget = backend.createTexture((uint32_t)viewportSize.x, (uint32_t)viewportSize.y, nullptr);

	// INIT FONT
	fontRenderer = backend.createFontRenderer();
	font24 = fontRenderer->createFont(Asset::path("font/Espera/Espera-Bold.ttf"), 24);
	font48 = fontRenderer->createFont(Asset::path("font/Espera/Espera-Bold.ttf"), 48);
	font96 = fontRenderer->createFont(Asset::path("font/Espera/Espera-Bold.ttf"), 96);

	// INIT FRAMEBUFFER
	glGenFramebuffers(1, &framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget->getID(), 0);
	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");

	window.setSizeLimits((int32_t)viewportSize.x, (int32_t)viewportSize.y, GLFW_DONT_CARE, GLFW_DONT_CARE);
	
	// INIT CHARACTER

	// IMGUI
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
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

	renderTarget->destroy();
	fontRenderer->destroyFont(font24);
	fontRenderer->destroyFont(font48);
	fontRenderer->destroyFont(font96);
	fontRenderer->destroy();
	background.animations[0].frames[0].texture->destroy();
	for(Sprite::Animation &animation: character.animations)
		for (Sprite::Frame& frame : animation.frames)
			frame.texture->destroy();
	glDeleteFramebuffers(1, &framebufferID);
}


void Game::update(GraphicBackend& backend)
{
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
		rigid->velocity = vec2f(0.f, 1.f);
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
						entity->add<Animator>(Animator(&colliderSprite, 0.f));
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
					ImGui::Image((void*)(uintptr_t)(map->texture->getID()), ImVec2(512, 512), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 0, 0, 1));

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
	m_world.update();
}
// TODO
// - Hide framebuffer impl
void Game::render(GraphicBackend& backend)
{
	backend.viewport(0, 0, (uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	// draw background
	m_world.render(backend);

	// Blit to main buffer
	uint32_t widthRatio = screenWidth() / (uint32_t)viewportSize.x;
	uint32_t heightRatio = screenHeight() / (uint32_t)viewportSize.y;
	uint32_t ratio = min(widthRatio, heightRatio);
	uint32_t scaledWidth = ratio * (uint32_t)viewportSize.x;
	uint32_t scaledHeight = ratio * (uint32_t)viewportSize.y;
	uint32_t w = (screenWidth() - scaledWidth) / 2;
	uint32_t h = (screenHeight() - scaledHeight) / 2;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	backend.viewport(0, 0, screenWidth(), screenHeight());
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, (GLint)viewportSize.x, (GLint)viewportSize.y, w, h, screenWidth() - w, screenHeight() - h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Draw text
	//backend.viewport(0, 0, screenWidth(), screenHeight());
	//fontRenderer->viewport(0, 0, screenWidth(), screenHeight());
	static float scale = 1.f;
	scale += input::scroll().y;
	const char* str = "Hello world !";
	vec2i size24 = font24.size(str);// *(int)scale;
	vec2i size48 = font48.size(str);// *(int)scale;
	vec2i size96 = font96.size(str);// *(int)scale;
	//fontRenderer->render(font48, str, (float)((int)screenWidth() / 2 - size24.x / 2), (float)((int)screenHeight() / 2 - size48.y / 2), 1.f, color3f(0.1f, 0.1f, 0.1f));
	//fontRenderer->render(font24, str, 10, (float)((int)screenHeight() / 2 - size24.y / 2), 1.f, color3f(0.1f, 0.1f, 0.1f));
	//fontRenderer->render(font96, str, (float)((int)screenWidth() / 2 - size96.x / 2) + size96.x + 10, (float)((int)screenHeight() / 2 - size96.y / 2), 1.f, color3f(0.1f, 0.1f, 0.1f));

	ASSERT((error = glGetError()) == GL_NO_ERROR, "");

	// Rendering imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}