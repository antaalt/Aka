#pragma once

#include "Application.h"

namespace aka {

// Module for third parties like ImGui ?
struct Module
{
	virtual void initialize(Window& window, GraphicBackend& backend) = 0;
	virtual void destroy(GraphicBackend& backend) = 0;
	virtual void update(GraphicBackend& backend) = 0;
	virtual void render(GraphicBackend& backend) = 0;
};

class Game : public Application
{
public:
	void initialize(Window& window, GraphicBackend& backend) override;
	void destroy(GraphicBackend& backend) override;
	void update(GraphicBackend& backend) override;
	void render(GraphicBackend& backend) override;
};

}

