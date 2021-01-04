#include "CustomApp.h"

#include "Platform.h"
#include "Image.h"
#include "Input.h"
#include "GLShader.h"

#include <iostream>

namespace app {

Texture* texture, *textureChar;
Shader *shader;
const uint32_t WIDTH = 567, HEIGHT = 388;
GLenum error = GL_NO_ERROR;

void CustomApp::initialize(Window& window, GraphicBackend& backend)
{
	// INIT TEXTURE BACKGROUND
	Image image = Image::load("data/textureBase.jpg");
	ASSERT(image.width == WIDTH, "incorrect width");
	ASSERT(image.height == HEIGHT, "incorrect height");
	texture = backend.createTexture(image.width, image.height, image.bytes.data());

	// INIT TEXTURE CHARACTER
	Image imageChar = Image::load("data/textureBase.jpg");
	ASSERT(imageChar.width == WIDTH, "incorrect width");
	ASSERT(imageChar.height == HEIGHT, "incorrect height");
	textureChar = backend.createTexture(imageChar.width, imageChar.height, imageChar.bytes.data());

	// INIT shaders
	ShaderInfo info {};
	info.vertex = backend.createShader(loadFromFile("./data/shaders/shader.vert").c_str(), ShaderType::VERTEX_SHADER);
	info.frag = backend.createShader(loadFromFile("./data/shaders/shader.frag").c_str(), ShaderType::FRAGMENT_SHADER);
	Uniform uniform;
	uniform.name = "u_Color";
	uniform.shaderType = ShaderType::FRAGMENT_SHADER;
	uniform.type = UniformType::Vec4;
	info.uniforms.push_back(uniform);
	shader = backend.createProgram(info);
	ASSERT((error = glGetError()) == GL_NO_ERROR, "");
}

void CustomApp::destroy(GraphicBackend& backend)
{
	delete texture;
	delete textureChar;
	delete shader;
}

void CustomApp::update(GraphicBackend& backend)
{
}

void CustomApp::render(GraphicBackend& backend)
{
	backend.clear();
	if (input::pressed(input::Key::Space))
		glClearColor(1.f, 0.f, 0.f, 1.f);
	else
		glClearColor(0.f, 0.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static int frame = 0;
	std::cout << "Frame " << frame++ << " " << input::pressed(input::Button::Button1) << std::endl;


	// INIT FRAMEBUFFER
	static GLuint framebufferID = 0;
	if(framebufferID == 0)
		glGenFramebuffers(1, &framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getID(), 0);

	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not create");

	backend.viewport(0, 0, screenWidth(), screenHeight());

	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, screenWidth() - 10, screenHeight() - 10, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// Draw texture to mainbuffer
	backend.viewport((int32_t)input::mouse().x, (int32_t)(screenHeight() - input::mouse().y), 50, 50);
	shader->use();
	if (input::pressed(input::Button::Button1))
		shader->set<vec4>("u_Color", vec4{ 0.0f, 1.f, 0.0f, 1.0f });
	else if (input::pressed(input::Button::Button2))
		shader->set<vec4>("u_Color", vec4{ 1.0f, 0.f, 0.0f, 1.0f });
	else
		shader->set<vec4>("u_Color", vec4{ 0.0f, 0.f, 1.0f, 1.0f });

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	ASSERT((error = glGetError()) == GL_NO_ERROR, "");
}

}