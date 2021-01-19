#include "Window.h"

#include "../Core/Application.h"
#include "Input.h"
#include "Logger.h"

namespace aka {

Window::Window(const Config& config) :
	m_app(config.app)
{
	glfwSetErrorCallback([](int error, const char* description) {
		Logger::error("[GLFW][", error, "] ", description);
	});
	if (glfwInit() != GLFW_TRUE)
		throw std::runtime_error("Could not init GLFW");

	// Backend API
	// TODO create backend depending on type
	switch (config.api)
	{
	case GraphicBackend::Api::OpenGL:
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
		break;
	default:
		throw std::runtime_error("Not implemented");
		break;
	}

	m_window = glfwCreateWindow(config.width, config.height, config.name.c_str(), NULL, NULL);
	if (m_window == NULL) {
		glfwTerminate();
		throw std::runtime_error("Could not init window");
	}
	glfwSetWindowUserPointer(m_window, this);
	glfwMakeContextCurrent(m_window); // Initialise GLEW

	// --- Callbacks ---
	// --- Size
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		Window* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
		w->m_app->resize(width, height);
		Logger::info("[GLFW] New window size : ", width, "x", height);
	});
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		Window* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
		w->m_backend.resize(width, height);
		Logger::info("[GLFW] New framebuffer size : ", width, " - ", height);
	});
	glfwSetWindowContentScaleCallback(m_window, [](GLFWwindow* window, float x, float y) {
		Logger::info("[GLFW] Content scaled : ", x, " - ", y);
	});
	glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow* window, int maximized) {
		// Called when window is maximized or unmaximized
		Logger::info("[GLFW] Maximized : ", maximized);
	});
	// --- Window
	glfwSetWindowFocusCallback(m_window, [](GLFWwindow* window, int focus) {
		Logger::info("[GLFW] Focus : ", focus);
		Window* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
	});
	glfwSetWindowRefreshCallback(m_window, [](GLFWwindow* window) {
		Logger::info("[GLFW] Window refresh");
	});
	glfwSetWindowIconifyCallback(m_window, [](GLFWwindow* window, int iconified) {
		Logger::info("[GLFW] Focus : ", iconified);
	});
	glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
		Logger::info("[GLFW] Closing window ");
	});
	// --- Monitor
	glfwSetMonitorCallback([](GLFWmonitor* monitor, int event) {
		// GLFW_CONNECTED or GLFW_DISCONNECTED
		Logger::info("[GLFW] Monitor event : ", event);
	});
	// --- Inputs
	input::initialize();
	initKeyboard();
	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mode) {
		// key : glfw keycode
		// scancode : os code
		// action : GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
		// mode : GLFW_MOD_SHIFT GLFW_MOD_CONTROL GLFW_MOD_ALT GLFW_MOD_SUPER GLFW_MOD_CAPS_LOCK GLFW_MOD_NUM_LOCK
		if (action == GLFW_PRESS)
			input::on_key_down(getKeyFromScancode(scancode));
		else if (action == GLFW_RELEASE)
			input::on_key_up(getKeyFromScancode(scancode));
		// TODO manage repeat ?
	});
	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mode) {
		if (action == GLFW_PRESS)
			input::on_mouse_button_down(static_cast<input::Button>(button));
		else if (action == GLFW_RELEASE)
			input::on_mouse_button_up(static_cast<input::Button>(button));
		// TODO manage repeat ?
	});
	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
		// position, in screen coordinates, relative to the upper-left corner of the client area of the window
		input::on_mouse_move(static_cast<float>(xpos), static_cast<float>(ypos));
	});
	glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
		input::on_mouse_scroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
	});
	glfwSetCursorEnterCallback(m_window, [](GLFWwindow* window, int entered) {
		// GLFW_TRUE if entered, GLFW_FALSE if left
		Logger::info("[GLFW] Cursor enter : ", entered);
	});
	glfwSetJoystickCallback([](int jid, int event) {
		// event : GLFW_CONNECTED, GLFW_DISCONNECTED
		Logger::info("[GLFW] Joystick event", event);
	});
	glfwSwapInterval(1); // 1 is vsync, 0 is free

	m_backend.initialize();

	m_app->initialize(*this, m_backend);
	m_backend.resize(config.width, config.height);
	m_app->resize(config.width, config.height);
}

Window::~Window()
{
	m_app->destroy(m_backend);
	m_backend.destroy();
	glfwTerminate();
}

void Window::loop()
{
	Time::Unit lastTick = Time::now();
	do {
		Time::Unit now = Time::now();
		Time::Unit deltaTime = min<Time::Unit>(now - lastTick, Time::Unit::milliseconds(100));
		lastTick = now;
		m_app->update(deltaTime);
		m_app->render(m_backend);
		input::update();
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	} while (!input::pressed(input::Key::Escape) && glfwWindowShouldClose(m_window) == 0);
}

GLFWwindow* Window::handle() const
{
	return m_window;
}

void Window::run(const Window::Config& config)
{
	Window window(config);
	window.loop();
}

void Window::setSizeLimits(int32_t minWidth, int32_t minHeight, int32_t maxWidth, int32_t maxHeight)
{
	glfwSetWindowSizeLimits(m_window, minWidth, minHeight, maxWidth, maxHeight);
}

}
