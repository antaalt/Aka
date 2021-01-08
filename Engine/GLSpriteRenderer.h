#pragma once

#include "SpriteRenderer.h"
#include "GLShader.h"

namespace app {
namespace gl {

class SpriteRenderer : public app::SpriteRenderer
{
public:
	void create() override;
	void destroy() override;
	void render(const Sprite& sprite) override;
private:
	Shader m_shader;
	GLuint m_vao;
};

}
};