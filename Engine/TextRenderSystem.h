#pragma once

#include "System.h"
#include "Shader.h"

namespace aka {

class TextRenderSystem : public System
{
public:
	TextRenderSystem(World* world);

	void draw(Batch &batch) override;
};

}

