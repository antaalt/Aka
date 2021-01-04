#pragma once

#include "Application.h"

namespace app {

class CustomApp : public Application
{
public:
	void initialize(Window& window, app::GraphicBackend& backend) override;
	void destroy(app::GraphicBackend& backend) override;
	void update(app::GraphicBackend& backend) override;
	void render(app::GraphicBackend& backend) override;
};

}

