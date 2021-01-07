#pragma once

#include "GLBackend.h"
#include "Texture.h"

namespace app {
namespace gl {

class Texture : public app::Texture
{
public:
	Texture();
	~Texture();
	void create(uint32_t width, uint32_t height, const void* data) override;
	void destroy() override;

	void bind() const override;

	ID getID() override;
private:
	uint32_t m_width;
	uint32_t m_height;
	GLuint m_textureID;
};

}
}
