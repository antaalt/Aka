#pragma once

#include <Aka/Platform/PlatformDevice.h>

#include <GLFW/glfw3.h>

namespace aka {

class PlatformGLFW3 : public PlatformDevice
{
public:
	PlatformGLFW3(const PlatformConfig& config);
	~PlatformGLFW3();

	void poll() override;
	void move(int32_t x, int32_t y) override;
	void resize(uint32_t width, uint32_t height) override;
	void setLimits(uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight) override;
	void fullscreen(bool enabled) override;
	GLFWwindow* getGLFW3Handle();
private:
	GLFWwindow* m_window;
};

};