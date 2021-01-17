#include "TextRenderSystem.h"

#include "Text.h"
#include "Transform2D.h"
#include "World.h"

#include "GLBackend.h"

namespace aka {

TextRenderSystem::TextRenderSystem(World* world) :
    System(world),
    m_vbo(0),
    m_vao(0)
{
}

void TextRenderSystem::create()
{
    // Shader
    ShaderInfo info{};
    info.vertex = Shader::create(Asset::loadString("shaders/font.vert").c_str(), ShaderType::VERTEX_SHADER);
    info.frag = Shader::create(Asset::loadString("shaders/font.frag").c_str(), ShaderType::FRAGMENT_SHADER);
    info.uniforms.push_back(Uniform{ UniformType::Vec4, ShaderType::FRAGMENT_SHADER, "textColor" });
    info.uniforms.push_back(Uniform{ UniformType::Mat4, ShaderType::VERTEX_SHADER, "projection" });
    m_shader.create(info);

    // m_vbo for drawing
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderSystem::destroy()
{
    m_shader.destroy();
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    m_vao = 0;
    m_vbo = 0;
}

void TextRenderSystem::update()
{
}

void TextRenderSystem::render(GraphicBackend& backend)
{
    m_world->each<Text, Transform2D>([&](Entity* entity, Text* text, Transform2D* transform) {
        // Enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_shader.use();
        m_shader.set<mat4f>("projection", mat4f::orthographic((float)backend.viewport().y, (float)backend.viewport().h, (float)backend.viewport().x, (float)backend.viewport().w, -1.f, 1.f));
        m_shader.set<color4f>("textColor", text->color);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(m_vao);

        // iterate through all characters
        float x = transform->position.x;
        float y = transform->position.y;
        float scale = transform->size.x;
        std::string::const_iterator c;
        for (const char& c : text->text)
        {
            // TODO check if rendering text out of screen for culling ?
            const Character& ch = text->font->characters[c];

            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;

            float w = ch.size.x * scale;
            float h = ch.size.y * scale;
            // update m_vbo for each character
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };
            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.texture->id());
            // update content of m_vbo memory
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    });
}

}