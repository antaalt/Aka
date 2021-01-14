#include "TileSystem.h"

#include "FileSystem.h"
#include "Graphic.h"
#include "Camera2D.h"
#include "Transform2D.h"
#include "Animator.h"
#include "World.h"

namespace app {

TileSystem::TileSystem(World* world) :
    System(world)
{
}

void TileSystem::create()
{
    // configure VAO/VBO
    GLuint vbo;
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(m_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    ShaderInfo info{};
    info.vertex = Shader::create(Asset::loadString("shaders/sprite.vert").c_str(), ShaderType::VERTEX_SHADER);
    info.frag = Shader::create(Asset::loadString("shaders/sprite.frag").c_str(), ShaderType::FRAGMENT_SHADER);
    info.uniforms.push_back(Uniform{ UniformType::Mat4, ShaderType::VERTEX_SHADER, "model" });
    info.uniforms.push_back(Uniform{ UniformType::Mat4, ShaderType::VERTEX_SHADER, "projection" });
    info.uniforms.push_back(Uniform{ UniformType::Vec3, ShaderType::FRAGMENT_SHADER, "spriteColor" });
    m_shader.create(info);
}

void TileSystem::destroy()
{
    m_shader.destroy();
}

void TileSystem::update()
{
}

void TileSystem::render(GraphicBackend& backend)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_world->each<Transform2D, Animator>([&](Entity *entity, Transform2D *transform, Animator* animator) {
        // prepare transformations
        m_shader.use();
        mat4f model = transform->model();

        m_shader.set<mat4f>("projection", mat4f::orthographic((float)backend.viewport().y, (float)backend.viewport().height, (float)backend.viewport().x, (float)backend.viewport().width, -1.f, 1.f));
        m_shader.set<mat4f>("model", model);
        m_shader.set<color3f>("spriteColor", color3f(1));

        glActiveTexture(GL_TEXTURE0);
        animator->getCurrentSpriteFrame().bind();

        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    });
}

}
