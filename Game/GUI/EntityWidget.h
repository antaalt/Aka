#pragma once
#include "GUINode.h"

namespace aka {

class EntityWidget : public GUIWidget
{
public:
	//void update() override;
	void draw(World& world, Resources& resources) override;
};

};