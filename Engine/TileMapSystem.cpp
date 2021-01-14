#include "TileMapSystem.h"

#include "FileSystem.h"
#include "Graphic.h"
#include "Camera2D.h"
#include "Transform2D.h"
#include "Animator.h"

namespace app {

/*void load()
{
    // create texture for atlas
    const OgmoLevel::Layer* background = m_currentLevel.getLayer("Background");
    const OgmoLevel::Layer* foreground = m_currentLevel.getLayer("Foreground");
    const OgmoLevel::Layer* playerground = m_currentLevel.getLayer("Playerground");
    ASSERT(background != nullptr, "");
    ASSERT(foreground != nullptr, "");
    ASSERT(playerground != nullptr, "");
    // Consider for now they are all the same.
    ASSERT(background->tileset == foreground->tileset, "");
    ASSERT(background->tileset == playerground->tileset, "");

    const OgmoLevel::Layer* layer = foreground;
}*/

void TileMapSystem::create()
{
    for (Entity* entity : m_entities)
    {
        TileMap* tileMap = entity->get<TileMap>();
        TileLayer* tileLayer = entity->get<TileLayer>();

        // Create layer UBO
        glGenBuffers(1, &tileLayer->ubo);

        glBindBuffer(GL_TEXTURE_BUFFER, tileLayer->ubo);
        glBufferData(GL_TEXTURE_BUFFER, tileLayer->tileID.size() * sizeof(int32_t), tileLayer->tileID.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);

        // Create layer TBO
        glGenTextures(1, &tileLayer->tbo);
        glBindTexture(GL_TEXTURE_BUFFER, tileLayer->tbo);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, tileLayer->ubo);
        glBindTexture(GL_TEXTURE_BUFFER, 0);

        glActiveTexture(GL_TEXTURE0);
    }
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
    info.uniforms.push_back(Uniform{ UniformType::SamplerBuffer, ShaderType::FRAGMENT_SHADER, "spriteIndices" });
    m_shader.create(info);

    checkError();
}

void TileMapSystem::destroy()
{
    m_shader.destroy();
}

void TileMapSystem::update()
{
}

void TileMapSystem::render(GraphicBackend &backend)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (Entity* entity : m_entities)
    {
        TileMap* atlas = entity->get<TileMap>();
        TileLayer* layer = entity->get<TileLayer>();
        Transform2D* transform = entity->get<Transform2D>();

        ASSERT(layer->gridSize == atlas->gridSize, "");

        m_shader.use();
        m_shader.set<mat4f>("projection", mat4f::orthographic((float)backend.viewport().y, (float)backend.viewport().height, (float)backend.viewport().x, (float)backend.viewport().width, -1.f, 1.f));
        m_shader.set<mat4f>("model", transform->model());
        m_shader.set<vec2u>("gridCountAtlas", atlas->gridCount);
        m_shader.set<vec2u>("gridCount", layer->gridCount);
        m_shader.set<color4f>("color", layer->color);
        m_shader.set<int32_t>("image", 0);
        m_shader.set<int32_t>("spriteIndices", 1);

        glActiveTexture(GL_TEXTURE0);
        atlas->texture->bind();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, layer->tbo);

        // Update data for new layer
        /*glBindBuffer(GL_TEXTURE_BUFFER, layer->ubo);
        void* data = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
        memcpy(data, layer->data.data(), layer->data.size() * sizeof(int32_t));
        bool done = glUnmapBuffer(GL_TEXTURE_BUFFER);*/

        glBindVertexArray(0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        checkError();
    }
}

bool TileMapSystem::valid(Entity* entity)
{
    if (!entity->has<TileLayer>()) return false;
    if (!entity->has<TileMap>()) return false;
    if (!entity->has<Transform2D>()) return false;
	return true;
}

}
