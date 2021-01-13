#include "SpriteAnimatorComponent.h"

#include "Graphic.h"

namespace app {

SpriteAnimatorComponent::SpriteAnimatorComponent() :
    m_vao(0)
{
}

SpriteAnimatorComponent::~SpriteAnimatorComponent()
{
    m_shader.destroy();
    if (m_vao != 0)
        glDeleteVertexArrays(1, &m_vao);
}
void SpriteAnimatorComponent::set(Sprite* sprite)
{
    m_sprite = sprite;
}

void SpriteAnimatorComponent::play(const std::string& animation)
{
    Sprite::Animation *a = m_sprite->getAnimation(animation);
    ASSERT(a != nullptr, "No valid animation");
    m_currentAnimation = static_cast<uint32_t>(a - m_sprite->animations.data());
    m_currentFrame = 0;
    m_animationStartTick = Time::now();
    m_currentAnimationDuration = a->duration();
}

Sprite::Frame& SpriteAnimatorComponent::getCurrentSpriteFrame() const
{
    return m_sprite->animations[m_currentAnimation].frames[m_currentFrame];
}

void SpriteAnimatorComponent::create(GraphicBackend& backend)
{
    // TODO move gl code to wrapper
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
    info.vertex = Shader::create(Asset::loadString("shaders/sprite.vert").c_str(), ShaderType::VERTEX_SHADER);
    info.frag = Shader::create(Asset::loadString("shaders/sprite.frag").c_str(), ShaderType::FRAGMENT_SHADER);
    info.uniforms.push_back(Uniform{ UniformType::Mat4, ShaderType::VERTEX_SHADER, "model" });
    info.uniforms.push_back(Uniform{ UniformType::Mat4, ShaderType::VERTEX_SHADER, "projection" });
    info.uniforms.push_back(Uniform{ UniformType::Vec3, ShaderType::FRAGMENT_SHADER, "spriteColor" });
    m_shader.create(info);
}

void SpriteAnimatorComponent::destroy(GraphicBackend& backend)
{
    m_shader.destroy();
    glDeleteVertexArrays(1, &m_vao);
    m_vao = 0;
}

void SpriteAnimatorComponent::update()
{
    // TODO Pass a global tick relative to the frame instead ?
    const Time::Unit now = Time::now();
    Time::Unit elapsedSincePlay = (now - m_animationStartTick) % m_currentAnimationDuration;
    Time::Unit currentFrameDuration = 0;
    uint32_t frameID = 0;
    while (elapsedSincePlay > (currentFrameDuration = m_sprite->animations[m_currentAnimation].frames[frameID].duration))
    {
        elapsedSincePlay -= currentFrameDuration;
        frameID = (frameID + 1) % m_sprite->animations[m_currentAnimation].frames.size();
    }
    m_currentFrame = frameID;
}

void SpriteAnimatorComponent::render(const Camera2D& camera, GraphicBackend& backend)
{
    // prepare transformations
    m_shader.use();
    mat4f model = mat4f::identity();
    model *= mat4f::translate(vec3f(-camera.position, 0.0f));
    model *= mat4f::translate(vec3f(m_sprite->position, 0.0f));
    model *= mat4f::translate(vec3f(0.5f * m_sprite->size.x, 0.5f * m_sprite->size.y, 0.0f));
    model *= mat4f::rotate(vec3f(0.0f, 0.0f, 1.0f), m_sprite->rotation);
    model *= mat4f::translate(vec3f(-0.5f * m_sprite->size.x, -0.5f * m_sprite->size.y, 0.0f));

    model *= mat4f::scale(vec3f(m_sprite->size, 1.0f));

    m_shader.set<mat4f>("projection", mat4f::orthographic((float)backend.viewport().y, (float)backend.viewport().height, (float)backend.viewport().x, (float)backend.viewport().width, -1.f, 1.f));
    m_shader.set<mat4f>("model", model);
    m_shader.set<color3f>("spriteColor", color3f(1));

    glActiveTexture(GL_TEXTURE0);
    getCurrentSpriteFrame().bind();

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

};