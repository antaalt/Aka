#include "Game.h"

#include "Platform.h"
#include "Image.h"
#include "Input.h"
#include "Time.h"
#include "System.h"
#include "SpriteAnimatorComponent.h"
#include "WorldComponent.h"
#include "PhysicSimulation.h"

#include <iostream>

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

PhysicSimulation physic(0.01f);
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
	}
	{
		StaticRectCollider2D *collider = physic.createStaticRectCollider();
		collider->rect.position = vec2f(192, 0);
		collider->rect.size = vec2f(128, 96);
	}
	{
		StaticRectCollider2D* collider = physic.createStaticRectCollider();
		collider->rect.position = vec2f(0);
		collider->rect.size = vec2f(320, 80);
	}
	physic.start();
}

void Game::destroy(GraphicBackend& backend)
{
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
}

}