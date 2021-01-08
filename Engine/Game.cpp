#include "Game.h"

#include "Platform.h"
#include "Image.h"
#include "Input.h"
#include "Time.h"
#include "System.h"
#include "SpriteAnimatorComponent.h"

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

const uint32_t BACKGROUND_WIDTH = 320, BACKGROUND_HEIGHT = 180;
const uint32_t CHAR_WIDTH = 16, CHAR_HEIGHT = 32;
GLenum error = GL_NO_ERROR;

vec2f charPosition;

void Game::initialize(Window& window, GraphicBackend& backend)
{
	{
		// INIT TEXTURE BACKGROUND
		Image image = Image::load(Asset::path("textures/background/background.png"));
		ASSERT(image.width == BACKGROUND_WIDTH, "incorrect width");
		ASSERT(image.height == BACKGROUND_HEIGHT, "incorrect height");

		Sprite::Animation animation;
		animation.name = "default";
		animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		background.animations.push_back(animation);
		background.position = vec2f(0);
		background.size = vec2f(BACKGROUND_WIDTH, BACKGROUND_HEIGHT);
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
		character.position = vec2f(BACKGROUND_WIDTH / 2.f, BACKGROUND_HEIGHT / 2.f);
		character.size = vec2f(CHAR_WIDTH, CHAR_HEIGHT);
		character.rotation = degreef(0);
		characterComponent.set(&character);
		characterComponent.create(backend);
		characterComponent.play("idle");
	}

	// INIT FRAMEBUFFER RENDER TARGET
	renderTarget = backend.createTexture(BACKGROUND_WIDTH, BACKGROUND_HEIGHT, nullptr);

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

	window.setSizeLimits(BACKGROUND_WIDTH, BACKGROUND_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void Game::destroy(GraphicBackend& backend)
{
	renderTarget->destroy();
	characterComponent.destroy(backend);
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
	character.position.x += input::pressed(input::Key::ArrowRight) - input::pressed(input::Key::ArrowLeft);
	character.position.y += input::pressed(input::Key::ArrowUp) - input::pressed(input::Key::ArrowDown);
	//character.rotation = degreef(Time::now() / 100.f);
	characterComponent.update();
	backgroundComponent.update();
}
// TODO
// - Hide framebuffer impl
void Game::render(GraphicBackend& backend)
{
	static uint32_t frame = 0;
	//std::cout << "FRAME " << frame++ << " " << Time::now() << std::endl;

	backend.viewport(0, 0, BACKGROUND_WIDTH, BACKGROUND_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	// draw background
	backgroundComponent.render(backend);
	characterComponent.render(backend);

	// Blit to main buffer
	uint32_t widthRatio = screenWidth() / BACKGROUND_WIDTH;
	uint32_t heightRatio = screenHeight() / BACKGROUND_HEIGHT;
	uint32_t ratio = min(widthRatio, heightRatio);
	uint32_t scaledWidth = ratio * BACKGROUND_WIDTH;
	uint32_t scaledHeight = ratio * BACKGROUND_HEIGHT;
	uint32_t w = (screenWidth() - scaledWidth) / 2;
	uint32_t h = (screenHeight() - scaledHeight) / 2;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	backend.viewport(0, 0, screenWidth(), screenHeight());
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, BACKGROUND_WIDTH, BACKGROUND_HEIGHT, w, h, screenWidth() - w, screenHeight() - h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Draw text
	backend.viewport(0, 0, screenWidth(), screenHeight());
	fontRenderer->viewport(0, 0, screenWidth(), screenHeight());
	static float scale = 1.f;
	scale += input::scroll().y;
	const char* str = "Hello world !";
	vec2i size24 = font24.size(str);// *(int)scale;
	vec2i size48 = font48.size(str);// *(int)scale;
	vec2i size96 = font96.size(str);// *(int)scale;
	fontRenderer->render(font48, str, (float)((int)screenWidth() / 2 - size24.x / 2), (float)((int)screenHeight() / 2 - size48.y / 2), 1.f, color3f(0.1f, 0.1f, 0.1f));
	fontRenderer->render(font24, str, 10, (float)((int)screenHeight() / 2 - size24.y / 2), 1.f, color3f(0.1f, 0.1f, 0.1f));
	fontRenderer->render(font96, str, (float)((int)screenWidth() / 2 - size96.x / 2) + size96.x + 10, (float)((int)screenHeight() / 2 - size96.y / 2), 1.f, color3f(0.1f, 0.1f, 0.1f));

	ASSERT((error = glGetError()) == GL_NO_ERROR, "");
}

}