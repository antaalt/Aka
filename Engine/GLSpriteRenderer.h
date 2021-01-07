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
	void render(const app::Texture& texture, const vec2f &position, const vec2f &size, radianf rotate, const color3f& color) override;
private:
	Shader m_shader;
	GLuint m_vao;
};

}
};