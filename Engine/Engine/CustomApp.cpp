#include "CustomApp.h"

#include "Platform.h"
#include "Image.h"
#include "Input.h"

#include <iostream>

namespace app {

void CustomApp::initialize(Window& window, GraphicBackend& backend)
{
}

void CustomApp::destroy(GraphicBackend& backend)
{
}

void CustomApp::update(GraphicBackend& backend)
{
}

void CustomApp::render(GraphicBackend& backend)
{
	//backend.clear();
	if (input::pressed(input::Key::Space))
		glClearColor(1.f, 0.f, 0.f, 1.f);
	else
		glClearColor(0.f, 0.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static int frame = 0;
	std::cout << "Frame " << frame++ << " " << input::pressed(input::Button::Button1) << std::endl;

	static const uint32_t WIDTH = 567, HEIGHT = 388;
	static const uint32_t SCREEN_WIDTH = 1280, SCREEN_HEIGHT = 720;

	// INIT FRAMEBUFFER
	static GLuint framebufferID = 0;
	if(framebufferID == 0)
		glGenFramebuffers(1, &framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

	// INIT TEXTURE BACKGROUND
	static GLuint textureID = 0;
	static const size_t size = 4 * WIDTH * HEIGHT;
	Image image;
	
	if (textureID == 0)
	{
		image = Image::load("data/textureBase.jpg");
		ASSERT(image.width == WIDTH, "incorrect width");
		ASSERT(image.height == HEIGHT, "incorrect height");
		glGenTextures(1, &textureID);
	}
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bytes.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	// INIT TEXTURE CHARACTER
	static int x = 0, y = 0;
	static GLuint textureCharID = 0;
	static const size_t sizeChar = 4 * WIDTH * HEIGHT;
	Image imageChar;
	if (textureCharID == 0)
	{
		imageChar = Image::load("data/textureBase.jpg");
		ASSERT(imageChar.width == WIDTH, "incorrect width");
		ASSERT(imageChar.height == HEIGHT, "incorrect height");
		glGenTextures(1, &textureCharID);
	}
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageChar.bytes.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Draw texture to framebuffer
	// use shaders


	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not create");

	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

}