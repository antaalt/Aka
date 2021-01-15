#pragma once

#include "System.h"
#include "Shader.h"

namespace aka {

class TextRenderSystem : public System
{
public:
	TextRenderSystem(World* world);

	void create() override;
	void destroy() override;

	void update() override;
	void render(GraphicBackend& backend) override;
private:
	Shader m_shader;
	uint32_t m_vbo, m_vao;
};

}

