#pragma once

#include "Application.h"

namespace app {

// Module for third parties like ImGui ?
struct Module
{
	virtual void initialize(Window& window, app::GraphicBackend& backend) = 0;
	virtual void destroy(app::GraphicBackend& backend) = 0;
	virtual void update(app::GraphicBackend& backend) = 0;
	virtual void render(app::GraphicBackend& backend) = 0;
};

class Game : public Application
{
public:
	void initialize(Window& window, app::GraphicBackend& backend) override;
	void destroy(app::GraphicBackend& backend) override;
	void update(app::GraphicBackend& backend) override;
	void render(app::GraphicBackend& backend) override;
};

}

