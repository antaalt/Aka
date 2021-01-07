#include "CustomApp.h"

#include "Platform.h"
#include "Image.h"
#include "Input.h"

#include <iostream>

namespace app {

GLuint framebufferID;
Texture* background, *character, *renderTarget;
FontRenderer* font;
SpriteRenderer* sprite;

const uint32_t WIDTH = 320, HEIGHT = 180;
const uint32_t CHAR_WIDTH = 20, CHAR_HEIGHT = 30;
GLenum error = GL_NO_ERROR;

vec2f charPosition;

void CustomApp::initialize(Window& window, GraphicBackend& backend)
{
	{
		// INIT TEXTURE BACKGROUND
		Image image = Image::load("../asset/textures/background.png");
		ASSERT(image.width == WIDTH, "incorrect width");
		ASSERT(image.height == HEIGHT, "incorrect height");
		background = backend.createTexture(image.width, image.height, image.bytes.data());
	}
	{
		// INIT SPRITE CHARACTER
		Image image = Image::load("../asset/textures/character.png");
		ASSERT(image.width == CHAR_WIDTH, "incorrect width");
		ASSERT(image.height == CHAR_HEIGHT, "incorrect height");
		character = backend.createTexture(image.width, image.height, image.bytes.data());
	}

	// INIT FRAMEBUFFER RENDER TARGET
	renderTarget = backend.createTexture(WIDTH, HEIGHT, nullptr);

	// INIT FONT
	font = backend.createFont("../asset/font/Espera-Bold.ttf");

	// INIT FRAMEBUFFER
	glGenFramebuffers(1, &framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget->getID(), 0);
	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");

	// INIT SPRITE RENDERING
	sprite = backend.createSprite();

	window.setSizeLimits(WIDTH, HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);
	charPosition.x = WIDTH / 2.f;
	charPosition.y = HEIGHT / 2.f;
}

void CustomApp::destroy(GraphicBackend& backend)
{
	renderTarget->destroy();
	background->destroy();
	character->destroy();
	sprite->destroy();
	font->destroy();
	glDeleteFramebuffers(1, &framebufferID);
}

void CustomApp::update(GraphicBackend& backend)
{
	charPosition.x += input::pressed(input::Key::ArrowRight) - input::pressed(input::Key::ArrowLeft);
	charPosition.y += input::pressed(input::Key::ArrowUp) - input::pressed(input::Key::ArrowDown);
}
// TODO
// - Add time component & animated sprite
// - SpriteRenderer & FontRenderer are both component. Create class Font & Sprite
// - Hide framebuffer impl
// - Asset class
// - IO class
void CustomApp::render(GraphicBackend& backend)
{
	static uint32_t frame = 0;
	std::cout << "FRAME " << frame++ << std::endl;


	backend.viewport(0, 0, WIDTH, HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	// draw background
	sprite->viewport(0, 0, WIDTH, HEIGHT);
	sprite->render(*background, vec2f(0), vec2f(WIDTH, HEIGHT), radianf(0), color3f(1,1,1));
	sprite->render(*character, charPosition, vec2f(CHAR_WIDTH, CHAR_HEIGHT), radianf(0), color3f(1, 1, 1));

	// Blit to main buffer
	uint32_t widthRatio = screenWidth() / WIDTH;
	uint32_t heightRatio = screenHeight() / HEIGHT;
	uint32_t ratio = min(widthRatio, heightRatio);
	uint32_t scaledWidth = ratio * WIDTH;
	uint32_t scaledHeight = ratio * HEIGHT;
	uint32_t w = (screenWidth() - scaledWidth) / 2;
	uint32_t h = (screenHeight() - scaledHeight) / 2;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	backend.viewport(0, 0, screenWidth(), screenHeight());
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, WIDTH, HEIGHT, w, h, screenWidth() - w, screenHeight() - h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Draw text
	backend.viewport(0, 0, screenWidth(), screenHeight());
	font->viewport(0, 0, screenWidth(), screenHeight());
	static float scale = 1.f;
	scale += input::scroll().y;
	const char* str = "Hello world !";
	vec2i size = font->size(str) * (int)scale;
	font->render(str, (float)((int)screenWidth() / 2 - size.x / 2), (float)((int)screenHeight() / 2 - size.y / 2), scale, color3f(0.1f, 0.1f, 0.1f));

	ASSERT((error = glGetError()) == GL_NO_ERROR, "");
}

}