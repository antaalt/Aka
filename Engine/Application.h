#pragma once

#include "GraphicBackend.h"
#include "Window.h"
#include "World.h"

namespace aka {

class Application
{
public:
	void resize(uint32_t width, uint32_t height) {
		m_screenWidth = width;
		m_screenHeight = height;
	}
	uint32_t screenWidth() const { return m_screenWidth; }
	uint32_t screenHeight() const { return m_screenHeight; }
public:
	// Initialize the app, set the listener on inputs, create resources...
	virtual void initialize(Window& window, GraphicBackend& backend) = 0;
	// Destroy everything related to the app.
	virtual void destroy(GraphicBackend& backend) = 0;
	// Update the app.
	virtual void update(Time::Unit deltaTime) = 0;
	// Render the app.
	virtual void render(GraphicBackend& backend) = 0;
private:
	uint32_t m_screenWidth, m_screenHeight;
protected:
	World m_world;
};

}