#pragma once

#include <Core/ECS/System.h>
#include <Graphic/GraphicBackend.h>

namespace aka {

class TextRenderSystem : public System
{
public:
	TextRenderSystem(World* world);

	void draw(Batch &batch) override;
};

}
