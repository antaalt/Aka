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
#include "TileSystem.h"

#include "OgmoWorld.h"

#include <sstream>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

namespace app {

GLuint framebufferID;
Sprite character;
Sprite background;
Sprite colliderSprite;
Texture* renderTarget;
FontRenderer* fontRenderer;
Font font24, font48, font96;

const uint32_t CHAR_WIDTH = 16, CHAR_HEIGHT = 32;
GLenum error = GL_NO_ERROR;

Entity* camera;
Entity* characterEntity;
Entity* backgroundEntity;
Entity* charCollider;
PhysicSystem *physicSystem;
AnimatorSystem *animatorSystem;
TileMapSystem* tileMapSystem;
TileSystem* tileSystem;

World world;


void Game::initialize(Window& window, GraphicBackend& backend)
{
	// TODO how do system get the entities ?
	physicSystem = world.createSystem<PhysicSystem>(Time::Unit(10));
	tileSystem = world.createSystem<TileSystem>();
	tileMapSystem = world.createSystem<TileMapSystem>();
	animatorSystem = world.createSystem<AnimatorSystem>();

	camera = world.createEntity();
	camera->add<Transform2D>(Transform2D(vec2f(0), vec2f(1), radianf(0)));
	Camera2D* c = camera->add<Camera2D>(Camera2D(vec2f(0), vec2f(320, 180)));
	{
		// INIT world
		OgmoWorld ogmoWorld = OgmoWorld::load(Asset::path("levels/world.ogmo"));
		OgmoLevel ogmoLevel = OgmoLevel::load(ogmoWorld, Asset::path("levels/level0.json"));
		const OgmoLevel::Layer* foreground = ogmoLevel.getLayer("Foreground");

		auto createTileLayer = [&](const OgmoLevel::Layer* layer, float depth) -> Entity* {
			if (layer->layer->type != OgmoWorld::LayerType::Tile)
				return nullptr;
			ASSERT(layer->tileset->tileSize == layer->gridCellSize, "");
			Entity* entity = world.createEntity();
			Texture* texture = backend.createTexture(layer->tileset->image.width, layer->tileset->image.height, layer->tileset->image.bytes.data());
			TileMap* tileMap = entity->add<TileMap>(TileMap(layer->tileset->tileCount, layer->tileset->tileSize, texture));
			TileLayer* tileLayer = entity->add<TileLayer>(TileLayer(layer->gridCellCount, layer->gridCellSize, color4f(1.f), layer->data, depth));
			Transform2D* transform = entity->add<Transform2D>(Transform2D(vec2f(0.f), vec2f(vec2u(layer->getWidth(), layer->getHeight())), radianf(0.f)));
			return entity;
		};
		createTileLayer(ogmoLevel.getLayer("Background"), 0.f);
		createTileLayer(ogmoLevel.getLayer("Playerground"), 0.f);
		createTileLayer(ogmoLevel.getLayer("Foreground"), 0.f);
	}
	{
		// INIT FIXED TEXTURE BACKGROUND
		Image image = Image::load(Asset::path("textures/background/background.png"));
		ASSERT(image.width == c->viewport.x, "incorrect width");
		ASSERT(image.height == c->viewport.y, "incorrect height");

		Sprite::Animation animation;
		animation.name = "default";
		animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		background.animations.push_back(animation);

		backgroundEntity = world.createEntity();
		backgroundEntity->add<Transform2D>(Transform2D(vec2f(0), c->viewport, radianf(0)));
		backgroundEntity->add<Animator>(Animator(&background, -2.f))->play("default");
	}
	{
		// INIT SPRITE CHARACTER
		// TODO move initialisation in SpriteAnimator & parse ase sprite directly ?
		// TODO load gif ?
		Sprite::Animation animation;
		animation.name = "idle";
		{
			Image image = Image::load(Asset::path("textures/player/player1.png"));
			animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		}
		{
			Image image = Image::load(Asset::path("textures/player/player2.png"));
			animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		}
		{
			Image image = Image::load(Asset::path("textures/player/player3.png"));
			animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		}
		character.animations.push_back(animation);
		characterEntity = world.createEntity();
		characterEntity->add<Transform2D>(Transform2D(c->viewport / 2.f, vec2f(CHAR_WIDTH, CHAR_HEIGHT), radianf(0)));
		characterEntity->add<Animator>(Animator(&character, 0.f))->play("idle");
	}
	{
		// INIT SPRITE COLLIDER
		Sprite::Animation animation;
		animation.name = "default";
		Image image = Image::load(Asset::path("textures/debug/collider.png"));
		animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		colliderSprite.animations.push_back(animation);
	}

	// INIT FRAMEBUFFER RENDER TARGET
	renderTarget = backend.createTexture((uint32_t)c->viewport.x, (uint32_t)c->viewport.y, nullptr);

	// INIT FONT
	fontRenderer = backend.createFontRenderer();
	font24 = fontRenderer->createFont(Asset::path("font/Espera-Bold.ttf"), 24);
	font48 = fontRenderer->createFont(Asset::path("font/Espera-Bold.ttf"), 48);
	font96 = fontRenderer->createFont(Asset::path("font/Espera-Bold.ttf"), 96);

	// INIT FRAMEBUFFER
	glGenFramebuffers(1, &framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget->getID(), 0);
	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");

	window.setSizeLimits((int32_t)c->viewport.x, (int32_t)c->viewport.y, GLFW_DONT_CARE, GLFW_DONT_CARE);
	
	// INIT CHARACTER
	charCollider = world.createEntity();
	charCollider->add<RigidBody2D>(RigidBody2D(vec2f(192, 160), 1.f, 0.1f, 0.1f));
	charCollider->add<Collider2D>(Collider2D(vec2f(192, 160), vec2f(16.f)));
	charCollider->add<Transform2D>(Transform2D(vec2f(192, 160), vec2f(16.f), radianf(0.f)));
	charCollider->add<Animator>(Animator(&colliderSprite, 0.f));

	// INIT COLLIDERS
	// TODO init from collider layer
	Entity* collider;

	collider = world.createEntity();
	collider->add<Collider2D>(Collider2D(vec2f(192, 0), vec2f(224, 96)));
	collider->add<Transform2D>(Transform2D(vec2f(192, 0), vec2f(224, 96), radianf(0.f)));
	collider->add<Animator>(Animator(&colliderSprite, 0.f));

	collider = world.createEntity();
	collider->add<Collider2D>(Collider2D(vec2f(0), vec2f(196, 80)));
	collider->add<Transform2D>(Transform2D(vec2f(0), vec2f(196, 80), radianf(0.f)));
	collider->add<Animator>(Animator(&colliderSprite, 0.f));

	collider = world.createEntity();
	collider->add<Collider2D>(Collider2D(vec2f(16, 96), vec2f(80, 64)));
	collider->add<Transform2D>(Transform2D(vec2f(16, 96), vec2f(80, 64), radianf(0.f)));
	collider->add<Animator>(Animator(&colliderSprite, 0.f));

	collider = world.createEntity();
	collider->add<Collider2D>(Collider2D(vec2f(96, 112), vec2f(16, 32)));
	collider->add<Transform2D>(Transform2D(vec2f(96, 112), vec2f(16, 32), radianf(0.f)));
	collider->add<Animator>(Animator(&colliderSprite, 0.f));

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
	world.create();
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

	Camera2D* c = camera->get<Camera2D>();

	c->position.x += input::pressed(input::Key::ArrowRight) - input::pressed(input::Key::ArrowLeft);
	c->position.y += input::pressed(input::Key::ArrowUp) - input::pressed(input::Key::ArrowDown);

	Transform2D* transform = charCollider->get<Transform2D>();
	RigidBody2D* rigid = charCollider->get<RigidBody2D>();
	Collider2D* collider = charCollider->get<Collider2D>();
	transform->position.x += input::pressed(input::Key::D) - input::pressed(input::Key::Q);
	transform->position.y += input::pressed(input::Key::Z) - input::pressed(input::Key::S);
	rigid->position.x += input::pressed(input::Key::D) - input::pressed(input::Key::Q);
	rigid->position.y += input::pressed(input::Key::Z) - input::pressed(input::Key::S);
	collider->position.x += input::pressed(input::Key::D) - input::pressed(input::Key::Q);
	collider->position.y += input::pressed(input::Key::Z) - input::pressed(input::Key::S);

	if (input::pressed(input::Key::Space))
	{
		rigid->acceleration = vec2f(0.f, 0.f);
		rigid->velocity = vec2f(0.f, 1.f);
	}
	if (input::pressed(input::Key::LeftCtrl))
	{
		transform->position = vec2f(192.f, 160.f);
		rigid->position = vec2f(192.f, 160.f);
		collider->position = vec2f(192.f, 160.f);
	}

	if (transform->position.y < -transform->size.y)
	{
		// teleport above
		transform->position = vec2f(transform->position.x, c->viewport.y);
		rigid->position = vec2f(rigid->position.x, c->viewport.y);
	}

	if (ImGui::Begin("Debug##window"))
	{
		ImVec4 color= ImVec4(236.f / 255.f, 11.f / 255.f, 67.f / 255.f, 1.f);
		if (ImGui::CollapsingHeader("Infos", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGuiIO &io = ImGui::GetIO();
			ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
			ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
			ImGui::InputFloat2("Camera", c->position.data, 3);
		}

		if (ImGui::CollapsingHeader("Colliders##header", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static bool renderColliders = true;
			ImGui::Checkbox("Render##checkbox", &renderColliders);
			char buffer[256];
			ImGui::TextColored(color, "Dynamics");
			for (Entity* entity : world)
			{
				// TODO Iterate on component family instead ?
				if (!entity->has<RigidBody2D>() || !entity->has<Collider2D>())
					continue;
				Collider2D* collider = entity->get<Collider2D>();
				RigidBody2D* rigid = entity->get<RigidBody2D>();
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
					ImGui::InputFloat2("Position", collider->position.data, 3);
					ImGui::InputFloat2("Size", collider->size.data, 3);

					ImGui::InputFloat("Mass", &rigid->mass, 0.1f, 1.f, 2);
					ImGui::InputFloat("Bouncing", &rigid->bouncing, 0.1f, 1.f, 2);
					ImGui::InputFloat("Friction", &rigid->friction, 0.1f, 1.f, 2);
					ImGui::InputFloat2("Acceleration", rigid->acceleration.data, 3);
					ImGui::InputFloat2("Velocity", rigid->velocity.data, 3);
					ImGui::TreePop();
				}
			}
			static int current_shape_dyn = 0;
			const char* current_shape_dyn_name[2] = {
				"Rect",
				"Circle"
			};
			if (ImGui::Button("Add##dynamic"))
			{
				if (current_shape_dyn == 0)
				{
					Entity* e = world.createEntity();
					e->add<Transform2D>(Transform2D(vec2f(0.f), vec2f(16.f), radianf(0.f)));
					e->add<Collider2D>(Collider2D(vec2f(0.f), vec2f(16.f)));
					e->add<RigidBody2D>(RigidBody2D(vec2f(0.f), 1.f));
				}
				else
				{

				}
			}
			ImGui::SameLine();
			ImGui::Combo("Shape##dynamic", &current_shape_dyn, current_shape_dyn_name, 2);
			ImGui::Separator();


			ImGui::TextColored(color, "Statics");
			for (Entity* entity: world)
			{
				if (!entity->has<Collider2D>())
					continue;
				Collider2D* collider = entity->get<Collider2D>();
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
					ImGui::InputFloat2("Position", collider->position.data, 3);
					ImGui::InputFloat2("Size", collider->size.data, 3);
					ImGui::TreePop();
				}
			}
			static int current_shape = 0;
			const char* current_shape_name[2] = {
				"Rect",
				"Circle"
			};
			if (ImGui::Button("Add"))
			{
				if (current_shape == 0)
				{
					Entity* e = world.createEntity();
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
			for (Entity* entity : world)
			{
				if (!entity->has<TileLayer>() || !entity->has<TileMap>())
					continue;
				TileLayer *layer = entity->get<TileLayer>();
				TileMap* map = entity->get<TileMap>();
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
					ImGui::Image((void*)(uintptr_t)(map->texture->getID()), ImVec2(512, 512), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1,1,1,1), ImVec4(1,0,0,1));

					static int id = 0;
					ImGui::SliderInt("", &id, 0, (int)layer->tileID.size() - 4);
					if (ImGui::InputInt4("", layer->tileID.data() + id))
					{
						// TODO update TBO
						//layer->tbo;
					}

					ImGui::TreePop();
				}
			}
		}
	}
	ImGui::End();

	// Update world after moving manually objects
	world.update();
}
// TODO
// - Hide framebuffer impl
void Game::render(GraphicBackend& backend)
{
	Camera2D* c = camera->get<Camera2D>();
	backend.viewport(0, 0, (uint32_t)c->viewport.x, (uint32_t)c->viewport.y);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	// draw background
	world.render(backend);

	// Blit to main buffer
	uint32_t widthRatio = screenWidth() / (uint32_t)c->viewport.x;
	uint32_t heightRatio = screenHeight() / (uint32_t)c->viewport.y;
	uint32_t ratio = min(widthRatio, heightRatio);
	uint32_t scaledWidth = ratio * (uint32_t)c->viewport.x;
	uint32_t scaledHeight = ratio * (uint32_t)c->viewport.y;
	uint32_t w = (screenWidth() - scaledWidth) / 2;
	uint32_t h = (screenHeight() - scaledHeight) / 2;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	backend.viewport(0, 0, screenWidth(), screenHeight());
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, (GLint)c->viewport.x, (GLint)c->viewport.y, w, h, screenWidth() - w, screenHeight() - h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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