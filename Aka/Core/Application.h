#pragma once

#include "../Graphic/GraphicBackend.h"
#include "../Platform/Window.h"
#include "ECS/World.h"

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
	virtual void initialize(Window& window) = 0;
	// Destroy everything related to the app.
	virtual void destroy() = 0;
	// Create a new frame for the app
	virtual void frame() = 0;
	// Update the app.
	virtual void update(Time::Unit deltaTime) = 0;
	// Render the app.
	virtual void render() = 0;
private:
	uint32_t m_screenWidth, m_screenHeight;
protected:
	World m_world;
};

}