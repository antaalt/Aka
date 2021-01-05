#include "GLFont.h"

namespace app {
namespace gl {

void Font::destroy()
{
    for (unsigned char c = 0; c < 128; c++)
        glDeleteTextures(1, &m_characters[c].textureID);

    m_shader.destroy();
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    m_vao = 0;
    m_vbo = 0;
}

void Font::render(const std::string& text, float x, float y, float scale, color3f color)
{
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_shader.use();
    m_shader.set<mat4f>("projection", mat4f::orthographic(m_y, m_height, m_x, m_width, -1.f, 1.f));
    m_shader.set<vec3f>("textColor", vec3f(color.x, color.y, color.z));

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_vao);

    // iterate through all characters
    std::string::const_iterator c;
    for (const char &c : text)
    {
        // TODO check if rendering text out of screen for culling ?
        const Character &ch = m_characters[c];

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
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
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
}

void Font::createBackend(FT_Face face)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        m_characters[c] = {
            vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top),
            texture,
            static_cast<uint32_t>(face->glyph->advance.x)
        };
    }

    // Shader
    ShaderInfo info{};
    info.vertex = gl::Shader::create(loadFromFile("../asset/shaders/font.vert").c_str(), ShaderType::VERTEX_SHADER);
    info.frag = gl::Shader::create(loadFromFile("../asset/shaders/font.frag").c_str(), ShaderType::FRAGMENT_SHADER);
    info.uniforms.push_back(Uniform{ UniformType::Vec3, ShaderType::FRAGMENT_SHADER, "textColor" });
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

};
};