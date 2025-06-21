#pragma once

#include <Aka/Platform/PlatformDevice.h>


struct GLFWwindow;

namespace aka {

class PlatformWindowGLFW3 : public PlatformWindow
{
public:
	PlatformWindowGLFW3(const PlatformWindowConfig& cfg);
	~PlatformWindowGLFW3();

	void poll() override;
	void move(int32_t x, int32_t y) override;
	void resize(uint32_t width, uint32_t height) override;
	void setLimits(uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight) override;
	void fullscreen(bool enabled) override;
	GLFWwindow* getGLFW3Handle();
	void* getNativeHandle();
protected:
	void initialize() override;
	void shutdown() override;
private:
	GLFWwindow* m_window;
};

class PlatformGLFW3 : public PlatformDevice
{
public:
	PlatformGLFW3();
	~PlatformGLFW3();

	void initialize() override;
	void shutdown() override;

	PlatformWindow* createWindow(const PlatformWindowConfig& cfg) override;
	void destroyWindow(PlatformWindow* window) override;
};

};