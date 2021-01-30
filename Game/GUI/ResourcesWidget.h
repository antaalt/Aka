#pragma once
#include "GUINode.h"

namespace aka {

class ResourcesWidget : public GUIWidget
{
public:
	//void update() override;
	void draw(World& world, Resources& resources) override;
};

};