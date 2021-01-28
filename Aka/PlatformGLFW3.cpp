#include "PlatformBackend.h"

#include "Platform/Logger.h"
#include "Core/Application.h"
#include "Graphic/GraphicBackend.h"
#include "Platform/InputBackend.h"

namespace aka {

struct GLFW3Context {
	GLFWwindow* window;
};

GLFW3Context ctx;

input::Key getKeyFromGLFW(int key)
{
	// TODO implement
	return input::Key::Space;
}

void PlatformBackend::initialize(const Config& config)
{
	glfwSetErrorCallback([](int error, const char* description) {
		Logger::error("[GLFW][", error, "] ", description);
	});
	if (glfwInit() != GLFW_TRUE)
		throw std::runtime_error("Could not init GLFW");

	// Backend API
#if defined(AKA_USE_OPENGL)
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if !defined(__APPLE__)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#else
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
#if defined(_DEBUG)
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
#elif defined(AKA_USE_D3D11)
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

	ctx.window = glfwCreateWindow(config.width, config.height, config.name.c_str(), NULL, NULL);
	if (ctx.window == nullptr) {
		glfwTerminate();
		throw std::runtime_error("Could not init window");
	}
#if defined(AKA_USE_OPENGL)
	glfwMakeContextCurrent(ctx.window);
	glfwSwapInterval(1); // 1 is vsync, 0 is free
#endif

	// --- Callbacks ---
	// --- Size
	glfwSetWindowSizeCallback(ctx.window, [](GLFWwindow* window, int width, int height) {
		Logger::info("[GLFW] New window size : ", width, "x", height);
	});
	glfwSetFramebufferSizeCallback(ctx.window, [](GLFWwindow* window, int width, int height) {
		GraphicBackend::resize(width, height);
		Logger::info("[GLFW] New framebuffer size : ", width, " - ", height);
	});
	glfwSetWindowContentScaleCallback(ctx.window, [](GLFWwindow* window, float x, float y) {
		Logger::info("[GLFW] Content scaled : ", x, " - ", y);
	});
	glfwSetWindowMaximizeCallback(ctx.window, [](GLFWwindow* window, int maximized) {
		// Called when window is maximized or unmaximized
		Logger::info("[GLFW] Maximized : ", maximized);
	});
	// --- Window
	glfwSetWindowFocusCallback(ctx.window, [](GLFWwindow* window, int focus) {
		Logger::info("[GLFW] Focus : ", focus);
	});
	glfwSetWindowRefreshCallback(ctx.window, [](GLFWwindow* window) {
		Logger::info("[GLFW] Window refresh");
	});
	glfwSetWindowIconifyCallback(ctx.window, [](GLFWwindow* window, int iconified) {
		Logger::info("[GLFW] Focus : ", iconified);
	});
	glfwSetWindowCloseCallback(ctx.window, [](GLFWwindow* window) {
		Logger::info("[GLFW] Closing window ");
	});
	// --- Monitor
	glfwSetMonitorCallback([](GLFWmonitor* monitor, int event) {
		// GLFW_CONNECTED or GLFW_DISCONNECTED
		Logger::info("[GLFW] Monitor event : ", event);
	});
	// --- Inputs
	glfwSetKeyCallback(ctx.window, [](GLFWwindow* window, int key, int scancode, int action, int mode) {
		// key : glfw keycode
		// scancode : os code
		// action : GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
		// mode : GLFW_MOD_SHIFT GLFW_MOD_CONTROL GLFW_MOD_ALT GLFW_MOD_SUPER GLFW_MOD_CAPS_LOCK GLFW_MOD_NUM_LOCK
		if (action == GLFW_PRESS)
			InputBackend::onKeyDown(InputBackend::getKeyFromScancode(scancode));
		else if (action == GLFW_RELEASE)
			InputBackend::onKeyUp(InputBackend::getKeyFromScancode(scancode));
		// TODO manage repeat ?
	});
	glfwSetMouseButtonCallback(ctx.window, [](GLFWwindow* window, int button, int action, int mode) {
		if (action == GLFW_PRESS)
			InputBackend::onMouseButtonDown(static_cast<input::Button>(button));
		else if (action == GLFW_RELEASE)
			InputBackend::onMouseButtonUp(static_cast<input::Button>(button));
		// TODO manage repeat ?
	});
	glfwSetCursorPosCallback(ctx.window, [](GLFWwindow* window, double xpos, double ypos) {
		// position, in screen coordinates, relative to the upper-left corner of the client area of the window
		InputBackend::onMouseMove(static_cast<float>(xpos), static_cast<float>(ypos));
	});
	glfwSetScrollCallback(ctx.window, [](GLFWwindow* window, double xoffset, double yoffset) {
		InputBackend::onMouseScroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
	});
	glfwSetCursorEnterCallback(ctx.window, [](GLFWwindow* window, int entered) {
		// GLFW_TRUE if entered, GLFW_FALSE if left
		Logger::info("[GLFW] Cursor enter : ", entered);
	});
	glfwSetJoystickCallback([](int jid, int event) {
		// event : GLFW_CONNECTED, GLFW_DISCONNECTED
		Logger::info("[GLFW] Joystick event", event);
	});
}
void PlatformBackend::destroy()
{
	glfwTerminate();
}
void PlatformBackend::frame()
{
	glfwPollEvents();
}
void PlatformBackend::present()
{
	if(GraphicBackend::api() == GraphicApi::OpenGL)
		glfwSwapBuffers(ctx.window);
}
bool PlatformBackend::running()
{
	return !glfwWindowShouldClose(ctx.window);
}

void PlatformBackend::getSize(uint32_t* width, uint32_t* height)
{
	glfwGetWindowSize(ctx.window, reinterpret_cast<int*>(width), reinterpret_cast<int*>(height));
}

void PlatformBackend::setSize(uint32_t width, uint32_t height)
{
	glfwSetWindowSize(ctx.window, width, height);
}

void PlatformBackend::setLimits(uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight)
{
	auto convert = [](uint32_t value) -> int { if (value == 0) return GLFW_DONT_CARE; return value; };
	glfwSetWindowSizeLimits(ctx.window, convert(minWidth), convert(minHeight), convert(maxWidth), convert(maxHeight));
}

GLFWwindow* PlatformBackend::getGLFW3Handle()
{
	return ctx.window;
}

#if defined(AKA_USE_D3D11)
HWND PlatformBackend::getD3DHandle()
{
	return glfwGetWin32Window(ctx.window);
}
#endif

};