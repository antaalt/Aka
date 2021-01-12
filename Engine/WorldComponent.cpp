#include "WorldComponent.h"

#include "Graphic.h"

namespace app {

GLuint ubo, tbo;

WorldComponent::WorldComponent()
{
}

void WorldComponent::loadWorld(const Path& worldPath)
{
	m_world = World::load(worldPath);
}

void WorldComponent::loadLevel(const Path& levelPath)
{
	// TODO destroy currentLevel
	m_currentLevel = Level::load(levelPath);
}

void WorldComponent::create(GraphicBackend& backend)
{
	// create texture for atlas
	const Level::Layer* background = m_currentLevel.getLayer("Background");
	const Level::Layer* foreground = m_currentLevel.getLayer("Foreground");
	const Level::Layer* playerground = m_currentLevel.getLayer("Playerground");
	ASSERT(background != nullptr, "");
	ASSERT(foreground != nullptr, "");
	ASSERT(playerground != nullptr, "");
	const World::Tileset *tileset = m_world.getTileset(background->tileset);
	// Consider for now they are all the same.
	ASSERT(tileset == m_world.getTileset(foreground->tileset), "");
	ASSERT(tileset == m_world.getTileset(playerground->tileset), "");

	// create texture for level
	m_atlas = backend.createTexture(tileset->image.width, tileset->image.height, tileset->image.bytes.data());

	// Create UBO
	glGenBuffers(1, &ubo);

	glBindBuffer(GL_TEXTURE_BUFFER, ubo);
	glBufferData(GL_TEXTURE_BUFFER, playerground->data.size() * sizeof(int32_t), playerground->data.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	
	// Create TBO
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_BUFFER, tbo);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, ubo);
	glBindTexture(GL_TEXTURE_BUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	checkError();

	// Create shader
	ShaderInfo info{};
	info.vertex = Shader::create(Asset::loadString("shaders/world.vert"), ShaderType::VERTEX_SHADER);
	info.frag = Shader::create(Asset::loadString("shaders/world.frag"), ShaderType::FRAGMENT_SHADER);
	info.uniforms.push_back(Uniform{ UniformType::Vec4, ShaderType::FRAGMENT_SHADER, "color" });
	info.uniforms.push_back(Uniform{ UniformType::Vec2, ShaderType::FRAGMENT_SHADER, "gridCountAtlas" });
	info.uniforms.push_back(Uniform{ UniformType::Vec2, ShaderType::FRAGMENT_SHADER, "gridCount" });
	info.uniforms.push_back(Uniform{ UniformType::Mat4, ShaderType::VERTEX_SHADER, "projection" });
	info.uniforms.push_back(Uniform{ UniformType::Mat4, ShaderType::VERTEX_SHADER, "model" });
	info.uniforms.push_back(Uniform{ UniformType::Sampler2D, ShaderType::FRAGMENT_SHADER, "image" });
	info.uniforms.push_back(Uniform{ UniformType::Sampler2D, ShaderType::FRAGMENT_SHADER, "spriteIndices" });
	m_shader.create(info);

	checkError();
}

void WorldComponent::destroy(GraphicBackend& backend)
{
	m_shader.destroy();
	m_atlas->destroy();
}

void WorldComponent::update()
{
}

void WorldComponent::render(GraphicBackend& backend)
{
	// Run for all layers (or select it before rendering)

	// Run shader with offset (for moving in the level)
	// Shader get with uv, the pixel position.
	// Knowing the grid size, we can retrieve in data buffer the index of the texture
	// With the index, we can retrieve the uv of the texture to get.

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const Level::Layer* background = m_currentLevel.getLayer("Playerground");
	const World::Tileset* atlas = m_world.getTileset(background->tileset);

	// Update it for offset
	vec2u scale = background->gridCellSize * background->gridCellCount;
	mat4f model = mat4f::identity();
	model *= mat4f::translate(vec3f(m_currentLevel.offset.x + background->offset.x, m_currentLevel.offset.y + background->offset.y, 0));
	model *= mat4f::scale(vec3f(scale.x, scale.y, 0U));
	
	m_shader.use();
	m_shader.set<mat4f>("projection", mat4f::orthographic((float)backend.viewport().y, (float)backend.viewport().height, (float)backend.viewport().x, (float)backend.viewport().width, -1.f, 1.f));
	m_shader.set<mat4f>("model", model);
	m_shader.set<vec2u>("gridCountAtlas", atlas->tileSize);
	m_shader.set<vec2u>("gridCount", background->gridCellCount);
	m_shader.set<color4f>("color", color4f(1.f));
	m_shader.set<int32_t>("image", 0);
	m_shader.set<int32_t>("spriteIndices", 1);

	glActiveTexture(GL_TEXTURE0);
	m_atlas->bind();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, tbo);

	glBindVertexArray(0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
	checkError();
}

};

