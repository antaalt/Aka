#pragma once

#include "GraphicBackend.h"
#include "Window.h"

namespace app {

class Application
{
public:
	// TODO add resize

	// Initialize the app, set the listener on inputs, create resources...
	virtual void initialize(Window& window, GraphicBackend& backend) = 0;
	// Destroy everything related to the app.
	virtual void destroy(GraphicBackend& backend) = 0;
	// Update the app.
	virtual void update(GraphicBackend& backend) = 0;
	// Render the app.
	virtual void render(GraphicBackend& backend) = 0;
};

}