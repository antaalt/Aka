#include "GLSpriteRenderer.h"

namespace app {
namespace gl {

void SpriteRenderer::create()
{
    // configure VAO/VBO
    unsigned int VBO;
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
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(m_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    ShaderInfo info{};
    info.vertex = Shader::create(loadFromFile("../asset/shaders/sprite.vert").c_str(), ShaderType::VERTEX_SHADER);
    info.frag = Shader::create(loadFromFile("../asset/shaders/sprite.frag").c_str(), ShaderType::FRAGMENT_SHADER);
    info.uniforms.push_back(Uniform{UniformType::Mat4, ShaderType::VERTEX_SHADER, "model"});
    info.uniforms.push_back(Uniform{ UniformType::Mat4, ShaderType::VERTEX_SHADER, "projection" });
    info.uniforms.push_back(Uniform{ UniformType::Vec3, ShaderType::FRAGMENT_SHADER, "spriteColor" });
    m_shader.create(info);
}

void SpriteRenderer::destroy()
{
    m_shader.destroy();
    glDeleteVertexArrays(1, &m_vao);
}

void SpriteRenderer::render(const app::Texture& texture, const vec2f& position, const vec2f& size, radianf rotate, const color3f& color)
{
    // prepare transformations
    m_shader.use();
    mat4f model = mat4f::identity();
    model *= mat4f::translate(vec3f(position, 0.0f));
    model *= mat4f::translate(vec3f(0.5f * size.x, 0.5f * size.y, 0.0f));
    model *= mat4f::rotate(vec3f(0.0f, 0.0f, 1.0f), rotate);
    model *= mat4f::translate(vec3f(-0.5f * size.x, -0.5f * size.y, 0.0f));

    model *= mat4f::scale(vec3f(size, 1.0f));

    m_shader.set<mat4f>("projection", mat4f::orthographic((float)m_y, (float)m_height, (float)m_x, (float)m_width, -1.f, 1.f));
    m_shader.set<mat4f>("model", model);
    m_shader.set<color3f>("spriteColor", color);

    glActiveTexture(GL_TEXTURE0);
    texture.bind();

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

};
};