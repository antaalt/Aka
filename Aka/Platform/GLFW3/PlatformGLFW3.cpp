#include "../PlatformBackend.h"

#include "../../OS/Logger.h"
#include "../../Core/Application.h"
#include "../../Graphic/GraphicBackend.h"
#include "../InputBackend.h"

#include <GLFW/glfw3.h>

namespace aka {

const input::Key glfwKeyMap[512] = {
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Space, // GLFW_KEY_SPACE              32
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown, // GLFW_KEY_APOSTROPHE         39
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown, // GLFW_KEY_COMMA              44
	input::Key::Unknown, // GLFW_KEY_MINUS              45
	input::Key::Unknown, // GLFW_KEY_PERIOD             46
	input::Key::Unknown, // GLFW_KEY_SLASH              47
	input::Key::Num0, // GLFW_KEY_0                  48
	input::Key::Num1, // GLFW_KEY_1                  49
	input::Key::Num2, // GLFW_KEY_2                  50
	input::Key::Num3, // GLFW_KEY_3                  51
	input::Key::Num4, // GLFW_KEY_4                  52
	input::Key::Num5, // GLFW_KEY_5                  53
	input::Key::Num6, // GLFW_KEY_6                  54
	input::Key::Num7, // GLFW_KEY_7                  55
	input::Key::Num8, // GLFW_KEY_8                  56
	input::Key::Num9, // GLFW_KEY_9                  57
	input::Key::Unknown, // GLFW_KEY_SEMICOLON          59
	input::Key::Unknown,
	input::Key::Unknown, // GLFW_KEY_EQUAL              61
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::A, // GLFW_KEY_A                  65
	input::Key::B, // GLFW_KEY_B                  66
	input::Key::C, // GLFW_KEY_C                  67
	input::Key::D, // GLFW_KEY_D                  68
	input::Key::E, // GLFW_KEY_E                  69
	input::Key::F, // GLFW_KEY_F                  70
	input::Key::G, // GLFW_KEY_G                  71
	input::Key::H, // GLFW_KEY_H                  72
	input::Key::I, // GLFW_KEY_I                  73
	input::Key::J, // GLFW_KEY_J                  74
	input::Key::K, // GLFW_KEY_K                  75
	input::Key::L, // GLFW_KEY_L                  76
	input::Key::M, // GLFW_KEY_M                  77
	input::Key::N, // GLFW_KEY_N                  78
	input::Key::O, // GLFW_KEY_O                  79
	input::Key::P, // GLFW_KEY_P                  80
	input::Key::Q, // GLFW_KEY_Q                  81
	input::Key::R, // GLFW_KEY_R                  82
	input::Key::S, // GLFW_KEY_S                  83
	input::Key::T, // GLFW_KEY_T                  84
	input::Key::U, // GLFW_KEY_U                  85
	input::Key::V, // GLFW_KEY_V                  86
	input::Key::W, // GLFW_KEY_W                  87
	input::Key::X, // GLFW_KEY_X                  88
	input::Key::Y, // GLFW_KEY_Y                  89
	input::Key::Z, // GLFW_KEY_Z                  90
	input::Key::Unknown, // GLFW_KEY_LEFT_BRACKET       91
	input::Key::Unknown, // GLFW_KEY_BACKSLASH          92
	input::Key::Unknown, // GLFW_KEY_RIGHT_BRACKET      93
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown, // GLFW_KEY_GRAVE_ACCENT       96
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown, // GLFW_KEY_WORLD_1            161
	input::Key::Unknown, // GLFW_KEY_WORLD_2            162
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Escape, // GLFW_KEY_ESCAPE             256
	input::Key::Enter, // GLFW_KEY_ENTER              257
	input::Key::Tab, // GLFW_KEY_TAB                258
	input::Key::BackSpace, // GLFW_KEY_BACKSPACE          259
	input::Key::Unknown, // GLFW_KEY_INSERT             260
	input::Key::Unknown, // GLFW_KEY_DELETE             261
	input::Key::ArrowRight, // GLFW_KEY_RIGHT              262
	input::Key::ArrowLeft, // GLFW_KEY_LEFT               263
	input::Key::ArrowDown, // GLFW_KEY_DOWN               264
	input::Key::ArrowUp, // GLFW_KEY_UP                 265
	input::Key::PageUp, // GLFW_KEY_PAGE_UP            266
	input::Key::PageDown, // GLFW_KEY_PAGE_DOWN          267
	input::Key::Home, // GLFW_KEY_HOME               268
	input::Key::End, // GLFW_KEY_END                269
	input::Key::CapsLock, // GLFW_KEY_CAPS_LOCK          280
	input::Key::Unknown, // GLFW_KEY_SCROLL_LOCK        281
	input::Key::NumLock, // GLFW_KEY_NUM_LOCK           282
	input::Key::PrintScreen, // GLFW_KEY_PRINT_SCREEN       283
	input::Key::Unknown, // GLFW_KEY_PAUSE              284
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::F1, // GLFW_KEY_F1                 290
	input::Key::F2, // GLFW_KEY_F2                 291
	input::Key::F3, // GLFW_KEY_F3                 292
	input::Key::F4, // GLFW_KEY_F4                 293
	input::Key::F5, // GLFW_KEY_F5                 294
	input::Key::F6, // GLFW_KEY_F6                 295
	input::Key::F7, // GLFW_KEY_F7                 296
	input::Key::F8, // GLFW_KEY_F8                 297
	input::Key::F9, // GLFW_KEY_F9                 298
	input::Key::F10, // GLFW_KEY_F10                299
	input::Key::F11, // GLFW_KEY_F11                300
	input::Key::F12, // GLFW_KEY_F12                301
	input::Key::Unknown, // GLFW_KEY_F13                302
	input::Key::Unknown, // GLFW_KEY_F14                303
	input::Key::Unknown, // GLFW_KEY_F15                304
	input::Key::Unknown, // GLFW_KEY_F16                305
	input::Key::Unknown, // GLFW_KEY_F17                306
	input::Key::Unknown, // GLFW_KEY_F18                307
	input::Key::Unknown, // GLFW_KEY_F19                308
	input::Key::Unknown, // GLFW_KEY_F20                309
	input::Key::Unknown, // GLFW_KEY_F21                310
	input::Key::Unknown, // GLFW_KEY_F22                311
	input::Key::Unknown, // GLFW_KEY_F23                312
	input::Key::Unknown, // GLFW_KEY_F24                313
	input::Key::Unknown, // GLFW_KEY_F25                314
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::NumPad0, // GLFW_KEY_KP_0               320
	input::Key::NumPad1, // GLFW_KEY_KP_1               321
	input::Key::NumPad2, // GLFW_KEY_KP_2               322
	input::Key::NumPad3, // GLFW_KEY_KP_3               323
	input::Key::NumPad4, // GLFW_KEY_KP_4               324
	input::Key::NumPad5, // GLFW_KEY_KP_5               325
	input::Key::NumPad6, // GLFW_KEY_KP_6               326
	input::Key::NumPad7, // GLFW_KEY_KP_7               327
	input::Key::NumPad8, // GLFW_KEY_KP_8               328
	input::Key::NumPad9, // GLFW_KEY_KP_9               329
	input::Key::Decimal, // GLFW_KEY_KP_DECIMAL         330
	input::Key::Divide, // GLFW_KEY_KP_DIVIDE          331
	input::Key::Multiply, // GLFW_KEY_KP_MULTIPLY        332
	input::Key::Substract, // GLFW_KEY_KP_SUBTRACT        333
	input::Key::Add, // GLFW_KEY_KP_ADD             334
	input::Key::Enter, // GLFW_KEY_KP_ENTER           335
	input::Key::Unknown, // GLFW_KEY_KP_EQUAL           336
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::LeftShift, // GLFW_KEY_LEFT_SHIFT         340
	input::Key::LeftCtrl, // GLFW_KEY_LEFT_CONTROL       341
	input::Key::LeftAlt, // GLFW_KEY_LEFT_ALT           342
	input::Key::Unknown, // GLFW_KEY_LEFT_SUPER         343
	input::Key::RightShift, // GLFW_KEY_RIGHT_SHIFT        344
	input::Key::RightCtrl, // GLFW_KEY_RIGHT_CONTROL      345
	input::Key::RightAlt, // GLFW_KEY_RIGHT_ALT          346
	input::Key::Unknown, // GLFW_KEY_RIGHT_SUPER        347
	input::Key::Unknown, // GLFW_KEY_MENU               348
};

struct GLFW3Context {
	GLFWwindow* window = nullptr;
};

GLFW3Context ctx;

input::Key InputBackend::getKeyFromPlatformKey(int key)
{
	return glfwKeyMap[key];
}

void PlatformBackend::initialize(const Config& config)
{
	glfwSetErrorCallback([](int error, const char* description) {
		Logger::error("[GLFW][", error, "] ", description);
	});
	if (glfwInit() != GLFW_TRUE)
		throw std::runtime_error("Could not init GLFW");

	// Backend API
	if (GraphicBackend::api() == GraphicApi::OpenGL)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if !defined(__APPLE__)
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#else
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
#if defined(DEBUG)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
	}
	else if (GraphicBackend::api() == GraphicApi::DirectX11)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	ctx.window = glfwCreateWindow(config.width, config.height, config.name.c_str(), NULL, NULL);
	if (ctx.window == nullptr) {
		glfwTerminate();
		throw std::runtime_error("Could not init window");
	}
#if defined(AKA_USE_OPENGL)
	glfwMakeContextCurrent(ctx.window);
	glfwSwapInterval(1); // default enable vsync
#endif

	// --- Callbacks ---
	// --- Size
	glfwSetWindowSizeCallback(ctx.window, [](GLFWwindow* window, int width, int height) {
		Logger::debug("[GLFW] New window size : ", width, "x", height);
	});
	glfwSetFramebufferSizeCallback(ctx.window, [](GLFWwindow* window, int width, int height) {
		GraphicBackend::resize(width, height);
		Logger::debug("[GLFW] New framebuffer size : ", width, " - ", height);
	});
	glfwSetWindowContentScaleCallback(ctx.window, [](GLFWwindow* window, float x, float y) {
		Logger::debug("[GLFW] Content scaled : ", x, " - ", y);
	});
	glfwSetWindowMaximizeCallback(ctx.window, [](GLFWwindow* window, int maximized) {
		// Called when window is maximized or unmaximized
		Logger::debug("[GLFW] Maximized : ", maximized);
	});
	// --- Window
	glfwSetWindowFocusCallback(ctx.window, [](GLFWwindow* window, int focus) {
		Logger::debug("[GLFW] Focus : ", focus);
	});
	glfwSetWindowRefreshCallback(ctx.window, [](GLFWwindow* window) {
		Logger::debug("[GLFW] Window refresh");
	});
	glfwSetWindowIconifyCallback(ctx.window, [](GLFWwindow* window, int iconified) {
		Logger::debug("[GLFW] Iconify : ", iconified);
	});
	glfwSetWindowCloseCallback(ctx.window, [](GLFWwindow* window) {
		Logger::debug("[GLFW] Closing window ");
	});
	// --- Monitor
	glfwSetMonitorCallback([](GLFWmonitor* monitor, int event) {
		// GLFW_CONNECTED or GLFW_DISCONNECTED
		Logger::debug("[GLFW] Monitor event : ", event);
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
		Logger::debug("[GLFW] Cursor enter : ", entered);
	});
	glfwSetJoystickCallback([](int jid, int event) {
		// event : GLFW_CONNECTED, GLFW_DISCONNECTED
		Logger::debug("[GLFW] Joystick event", event);
	});
}

void PlatformBackend::destroy()
{
	glfwTerminate();
}

void PlatformBackend::update()
{
	glfwPollEvents();
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

};
