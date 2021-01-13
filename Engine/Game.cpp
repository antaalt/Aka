#include "Game.h"

#include "Platform.h"
#include "Image.h"
#include "Input.h"
#include "Time.h"
#include "Logger.h"
#include "System.h"
#include "SpriteAnimatorComponent.h"
#include "WorldComponent.h"
#include "PhysicSimulation.h"

#include <sstream>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

namespace app {

GLuint framebufferID;
Sprite character;
Sprite background;
SpriteAnimatorComponent characterComponent;
SpriteAnimatorComponent backgroundComponent;
Texture* renderTarget;
FontRenderer* fontRenderer;
Font font24, font48, font96;
WorldComponent worldComponent;

const uint32_t CHAR_WIDTH = 16, CHAR_HEIGHT = 32;
GLenum error = GL_NO_ERROR;

Camera2D camera;

PhysicSimulation physic(Time::Unit::milliseconds(10));
DynamicRectCollider2D* charCollider;


void Game::initialize(Window& window, GraphicBackend& backend)
{
	camera.position = vec2f(0);
	camera.viewport = vec2f(320, 180);
	{
		// INIT world
		worldComponent.loadWorld(Asset::path("levels/world.ogmo"));
		worldComponent.loadLevel(Asset::path("levels/level0.json"));
		worldComponent.create(backend);
	}
	{
		// INIT FIXED TEXTURE BACKGROUND
		Image image = Image::load(Asset::path("textures/background/background.png"));
		ASSERT(image.width == camera.viewport.x, "incorrect width");
		ASSERT(image.height == camera.viewport.y, "incorrect height");

		Sprite::Animation animation;
		animation.name = "default";
		animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		background.animations.push_back(animation);
		background.position = vec2f(0);
		background.size = camera.viewport;
		background.rotation = radianf(0);
		
		backgroundComponent.set(&background);
		backgroundComponent.create(backend);
		backgroundComponent.play("default");
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
		character.position = camera.viewport / 2.f;
		character.size = vec2f(CHAR_WIDTH, CHAR_HEIGHT);
		character.rotation = degreef(0);
		characterComponent.set(&character);
		characterComponent.create(backend);
		characterComponent.play("idle");
	}

	// INIT FRAMEBUFFER RENDER TARGET
	renderTarget = backend.createTexture((uint32_t)camera.viewport.x, (uint32_t)camera.viewport.y, nullptr);

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

	window.setSizeLimits((int32_t)camera.viewport.x, (int32_t)camera.viewport.y, GLFW_DONT_CARE, GLFW_DONT_CARE);

	physic.create(backend);
	{
		charCollider = physic.createDynamicRectCollider();
		charCollider->rect.position = vec2f(192, 160);
		charCollider->rect.size = vec2f(16);
		charCollider->mass = 1.f;
		charCollider->bouncing = 0.1f;
		charCollider->friction = 0.1f;
	}
	{
		StaticRectCollider2D *collider = physic.createStaticRectCollider();
		collider->rect.position = vec2f(192, 0);
		collider->rect.size = vec2f(224, 96);
		collider->bouncing = 0.f;
		collider->friction = 0.f;
	}
	{
		StaticRectCollider2D* collider = physic.createStaticRectCollider();
		collider->rect.position = vec2f(0);
		collider->rect.size = vec2f(196, 80);
		collider->bouncing = 0.f;
		collider->friction = 0.f;
	}
	{
		StaticRectCollider2D* collider = physic.createStaticRectCollider();
		collider->rect.position = vec2f(16, 96);
		collider->rect.size = vec2f(80, 64);
		collider->bouncing = 0.f;
		collider->friction = 0.f;
	}
	{
		StaticRectCollider2D* collider = physic.createStaticRectCollider();
		collider->rect.position = vec2f(96, 112);
		collider->rect.size = vec2f(16, 32);
		collider->bouncing = 0.f;
		collider->friction = 0.f;
	}
	physic.start();

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
}

void Game::destroy(GraphicBackend& backend)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	renderTarget->destroy();
	characterComponent.destroy(backend);
	worldComponent.destroy(backend);
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

	camera.position.x += input::pressed(input::Key::ArrowRight) - input::pressed(input::Key::ArrowLeft);
	camera.position.y += input::pressed(input::Key::ArrowUp) - input::pressed(input::Key::ArrowDown);
	characterComponent.update();
	backgroundComponent.update();
	Shape2D* shape = charCollider->getShape();
	float x = shape->getPosition().x + input::pressed(input::Key::D) - input::pressed(input::Key::Q);
	float y = shape->getPosition().y + input::pressed(input::Key::Z) - input::pressed(input::Key::S);
	shape->setPosition(vec2f(x, y));

	if (input::pressed(input::Key::Space))
	{
		charCollider->acceleration = vec2f(0.f, 0.f);
		charCollider->velocity = vec2f(0.f, 1.f);

		//charCollider->acceleration = vec2f(0.f, 0.f);
		//charCollider->velocity = vec2f(0.f, 0.f);
		//charCollider->position = vec2f(192.f, 160.f);
	}
	if (input::pressed(input::Key::LeftCtrl))
	{
		charCollider->getShape()->setPosition(vec2f(192.f, 160.f));
	}

	if (charCollider->getShape()->getPosition().y < -charCollider->getShape()->getSize().y)
	{
		// teleport above
		charCollider->getShape()->setPosition(vec2f(charCollider->getShape()->getPosition().x, camera.viewport.y));
	}

	if (ImGui::Begin("Debug##window"))
	{
		ImVec4 color= ImVec4(236.f / 255.f, 11.f / 255.f, 67.f / 255.f, 1.f);
		if (ImGui::CollapsingHeader("Infos", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGuiIO &io = ImGui::GetIO();
			ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
			ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
			ImGui::InputFloat2("Camera", camera.position.data, 3);
		}

		if (ImGui::CollapsingHeader("Colliders##header", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("Render##checkbox", &physic.renderColliders);
			char buffer[256];
			ImGui::TextColored(color, "Dynamics");
			for (DynamicCollider2D* dynamic : physic.dynamics)
			{
				Shape2D::Type type = dynamic->getShape()->getType();
				std::string typeName = Shape2D::getTypeName(type);
				snprintf(buffer, 256, "Dynamic%s##%p", typeName.c_str(), dynamic);
				if (ImGui::TreeNode(buffer))
				{
					switch (type)
					{
					case Shape2D::Type::Rect: {
						Rect* rect = reinterpret_cast<Rect*>(dynamic->getShape());
						ImGui::InputFloat2("Position", rect->position.data, 3);
						ImGui::InputFloat2("Size", rect->size.data, 3);
						break;
					}
					/*case Shape2D::Type::Circle: {
						Circle* circle = reinterpret_cast<Circle*>(dynamic->getShape());
						ImGui::InputFloat2("Position", circle->position.data, 3);
						ImGui::InputFloat("Radius", circle->radius, 0.1f, 1.f, 2);
						break;
					}*/
					default:
						break;
					}
					ImGui::InputFloat("Mass", &dynamic->mass, 0.1f, 1.f, 2);
					ImGui::InputFloat("Bouncing", &dynamic->bouncing, 0.1f, 1.f, 2);
					ImGui::InputFloat("Friction", &dynamic->friction, 0.1f, 1.f, 2);
					ImGui::InputFloat2("Acceleration", dynamic->acceleration.data, 3);
					ImGui::InputFloat2("Velocity", dynamic->velocity.data, 3);
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
					DynamicRectCollider2D* c = physic.createDynamicRectCollider();
					c->rect.position = vec2f(0.f);
					c->rect.size = vec2f(16.f);
				}
				else
				{

				}
			}
			ImGui::SameLine();
			ImGui::Combo("Shape##dynamic", &current_shape_dyn, current_shape_dyn_name, 2);
			ImGui::Separator();


			ImGui::TextColored(color, "Statics");
			for (StaticCollider2D* staticc : physic.statics)
			{
				Shape2D::Type type = staticc->getShape()->getType();
				std::string typeName = Shape2D::getTypeName(type);
				snprintf(buffer, 256, "Static%s##%p", typeName.c_str(), staticc);
				if (ImGui::TreeNode(buffer))
				{
					switch (type)
					{
					case Shape2D::Type::Rect: {
						Rect* rect = reinterpret_cast<Rect*>(staticc->getShape());
						ImGui::InputFloat2("Position", rect->position.data, 3);
						ImGui::InputFloat2("Size", rect->size.data, 3);
						break;
					}
					default:
						break;
					}
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
					StaticRectCollider2D* c = physic.createStaticRectCollider();
					c->rect.position = vec2f(0.f);
					c->rect.size = vec2f(16.f);
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
			Level &level = worldComponent.getCurrentLevel();
			ImGui::TextColored(color, "Layers");
			static const char* layerTypeName[] = {
				"Tile",
				"Grid",
				"Entity"
			};
			for (Level::Layer& layer : level.layers)
			{
				if (ImGui::TreeNode(layer.layer->name.c_str()))
				{
					ImGui::Text("%s", layerTypeName[(int)layer.layer->type]);
					vec2i gridCellCount = vec2i(layer.gridCellCount);
					vec2i gridCellSize = vec2i(layer.gridCellSize);
					if (ImGui::InputInt2("Grid count", gridCellCount.data))
						layer.gridCellCount = vec2u(gridCellCount);
					if (ImGui::InputInt2("Grid size", gridCellSize.data))
						layer.gridCellSize = vec2u(gridCellSize);
					
					switch (layer.layer->type)
					{
					case World::LayerType::Tile:
						//ImGui::Image(layer.tileset->image);
						layer.data;
						break;
					case World::LayerType::Grid:
						break;
					case World::LayerType::Entity:
						char buffer[256];
						uint32_t id = 0;
						for (Level::Entity& entity : layer.entities)
						{
							snprintf(buffer, 256, "%s%2u", entity.entity->name.c_str(), id++);
							if (ImGui::TreeNode(buffer))
							{
								entity.entity->image;
								vec2i origin = vec2i(entity.entity->origin);
								vec2i tileSize = vec2i(entity.entity->size);
								if (ImGui::InputInt2("Origin", origin.data))
									const_cast<vec2u&>(entity.entity->origin) = vec2u(origin);
								if (ImGui::InputInt2("Tile size", tileSize.data))
									const_cast<vec2u&>(entity.entity->origin) = vec2u(tileSize);

								vec2i position = vec2i(entity.position);
								vec2i size = vec2i(entity.size);
								if (ImGui::InputInt2("Position", position.data))
									entity.position = vec2u(position);
								if (ImGui::InputInt2("Size", size.data))
									entity.size = vec2u(size);
								ImGui::TreePop();
							}
						}
						break;
					}
					ImGui::TreePop();
				}
			}
		}
	}
	ImGui::End();

	// Update physic after moving manually objects
	physic.update();
}
// TODO
// - Hide framebuffer impl
void Game::render(GraphicBackend& backend)
{
	backend.viewport(0, 0, (uint32_t)camera.viewport.x, (uint32_t)camera.viewport.y);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	// draw background
	backgroundComponent.render(camera, backend);
	worldComponent.render(camera, backend);
	characterComponent.render(camera, backend);

	physic.render(camera, backend);

	// Blit to main buffer
	uint32_t widthRatio = screenWidth() / (uint32_t)camera.viewport.x;
	uint32_t heightRatio = screenHeight() / (uint32_t)camera.viewport.y;
	uint32_t ratio = min(widthRatio, heightRatio);
	uint32_t scaledWidth = ratio * (uint32_t)camera.viewport.x;
	uint32_t scaledHeight = ratio * (uint32_t)camera.viewport.y;
	uint32_t w = (screenWidth() - scaledWidth) / 2;
	uint32_t h = (screenHeight() - scaledHeight) / 2;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	backend.viewport(0, 0, screenWidth(), screenHeight());
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, (GLint)camera.viewport.x, (GLint)camera.viewport.y, w, h, screenWidth() - w, screenHeight() - h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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