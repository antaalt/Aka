#pragma once

#include "../Core/System.h"
#include "../Graphic/Shader.h"

namespace aka {

class TextRenderSystem : public System
{
public:
	TextRenderSystem(World* world);

	void draw(Batch &batch) override;
};

}

