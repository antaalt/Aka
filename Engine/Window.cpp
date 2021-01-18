#include "Window.h"

#include "GraphicBackend.h"
#include "Application.h"
#include "Input.h"

namespace aka {

void setInputCallback(GLFWwindow* window)
{
	input::initialize();
	initKeyboard();
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mode) {
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
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mode) {
		if (action == GLFW_PRESS)
			input::on_mouse_button_down(static_cast<input::Button>(button));
		else if (action == GLFW_RELEASE)
			input::on_mouse_button_up(static_cast<input::Button>(button));
		// TODO manage repeat ?
	});
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
		// position, in screen coordinates, relative to the upper-left corner of the client area of the window
		input::on_mouse_move(static_cast<float>(xpos), static_cast<float>(ypos));
	});
	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		input::on_mouse_scroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
	});
	glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered) {
		// GLFW_TRUE if entered, GLFW_FALSE if left
	});
	glfwSetJoystickCallback([](int jid, int event) {
		// event : GLFW_CONNECTED, GLFW_DISCONNECTED
	});
}

Window::Window(const Config& config) :
	m_app(config.app),
	m_width(config.width),
	m_height(config.height)
{
	glfwSetErrorCallback([](int error, const char* description) {
		std::cout << "[GLFW][" << error << "] " << description << std::endl;
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
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		Window* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
		w->resize(width, height);
	});
	glfwSwapInterval(1); // 1 is vsync, 0 is free
	setInputCallback(m_window);

	m_backend.initialize();

	m_app->initialize(*this, m_backend);
	m_backend.resize(m_width, m_height);
	m_app->resize(m_width, m_height);
}

Window::~Window()
{
	m_app->destroy(m_backend);
	m_backend.destroy();
	glfwTerminate();
}

void Window::resize(uint32_t width, uint32_t height)
{
	m_backend.resize(width, height);
	m_app->resize(width, height);
}

void Window::loop()
{
	Time::Unit lastTick = Time::now();
	do {
		Time::Unit now = Time::now();
		Time::Unit deltaTime = now - lastTick;
		lastTick = now;
		m_app->update(deltaTime);
		m_app->render(m_backend);
		input::update();
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	} while (!input::pressed(input::Key::Escape) && glfwWindowShouldClose(m_window) == 0);
}

void* Window::getHandle() const
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

vec2i Window::getBackbufferSize() const
{
	vec2i size;
	glfwGetFramebufferSize(m_window, &size.x, &size.y);
	return size;
}

}
