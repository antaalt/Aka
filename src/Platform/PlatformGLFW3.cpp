#include <Aka/Platform/PlatformBackend.h>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Application.h>
#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/Platform/InputBackend.h>

#include <GLFW/glfw3.h>
#if defined(AKA_PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

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
	input::Key::Apostrophe, // GLFW_KEY_APOSTROPHE         39
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Comma, // GLFW_KEY_COMMA              44
	input::Key::Minus, // GLFW_KEY_MINUS              45
	input::Key::Period, // GLFW_KEY_PERIOD             46
	input::Key::Slash, // GLFW_KEY_SLASH              47
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
	input::Key::Unknown,
	input::Key::Semicolon, // GLFW_KEY_SEMICOLON          59
	input::Key::Unknown,
	input::Key::Equal, // GLFW_KEY_EQUAL              61
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
	input::Key::BracketLeft, // GLFW_KEY_LEFT_BRACKET       91
	input::Key::BackSlash, // GLFW_KEY_BACKSLASH          92
	input::Key::BracketRight, // GLFW_KEY_RIGHT_BRACKET      93
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Grave, // GLFW_KEY_GRAVE_ACCENT       96
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
	input::Key::Insert, // GLFW_KEY_INSERT             260
	input::Key::Delete, // GLFW_KEY_DELETE             261
	input::Key::ArrowRight, // GLFW_KEY_RIGHT              262
	input::Key::ArrowLeft, // GLFW_KEY_LEFT               263
	input::Key::ArrowDown, // GLFW_KEY_DOWN               264
	input::Key::ArrowUp, // GLFW_KEY_UP                 265
	input::Key::PageUp, // GLFW_KEY_PAGE_UP            266
	input::Key::PageDown, // GLFW_KEY_PAGE_DOWN          267
	input::Key::Home, // GLFW_KEY_HOME               268
	input::Key::End, // GLFW_KEY_END                269
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
	input::Key::CapsLock, // GLFW_KEY_CAPS_LOCK          280
	input::Key::ScrollLock, // GLFW_KEY_SCROLL_LOCK        281
	input::Key::NumLock, // GLFW_KEY_NUM_LOCK           282
	input::Key::PrintScreen, // GLFW_KEY_PRINT_SCREEN       283
	input::Key::Pause, // GLFW_KEY_PAUSE              284
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
	input::Key::KeyPad0, // GLFW_KEY_KP_0               320
	input::Key::KeyPad1, // GLFW_KEY_KP_1               321
	input::Key::KeyPad2, // GLFW_KEY_KP_2               322
	input::Key::KeyPad3, // GLFW_KEY_KP_3               323
	input::Key::KeyPad4, // GLFW_KEY_KP_4               324
	input::Key::KeyPad5, // GLFW_KEY_KP_5               325
	input::Key::KeyPad6, // GLFW_KEY_KP_6               326
	input::Key::KeyPad7, // GLFW_KEY_KP_7               327
	input::Key::KeyPad8, // GLFW_KEY_KP_8               328
	input::Key::KeyPad9, // GLFW_KEY_KP_9               329
	input::Key::KeyPadDecimal, // GLFW_KEY_KP_DECIMAL         330
	input::Key::KeyPadDivide, // GLFW_KEY_KP_DIVIDE          331
	input::Key::KeyPadMultiply, // GLFW_KEY_KP_MULTIPLY        332
	input::Key::KeyPadSubstract, // GLFW_KEY_KP_SUBTRACT        333
	input::Key::KeyPadAdd, // GLFW_KEY_KP_ADD             334
	input::Key::KeyPadEnter, // GLFW_KEY_KP_ENTER           335
	input::Key::Unknown, // GLFW_KEY_KP_EQUAL           336
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::ShiftLeft, // GLFW_KEY_LEFT_SHIFT         340
	input::Key::ControlLeft, // GLFW_KEY_LEFT_CONTROL       341
	input::Key::AltLeft, // GLFW_KEY_LEFT_ALT           342
	input::Key::Unknown, // GLFW_KEY_LEFT_SUPER         343
	input::Key::ShiftRight, // GLFW_KEY_RIGHT_SHIFT        344
	input::Key::ControlRight, // GLFW_KEY_RIGHT_CONTROL      345
	input::Key::AltRight, // GLFW_KEY_RIGHT_ALT          346
	input::Key::Unknown, // GLFW_KEY_RIGHT_SUPER        347
	input::Key::Menu, // GLFW_KEY_MENU               348
};

struct GLFW3Context {
	GLFWwindow* window = nullptr;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
};

GLFW3Context pctx;

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

	pctx.window = glfwCreateWindow(config.width, config.height, config.name.c_str(), NULL, NULL);
	glfwGetWindowSize(pctx.window, reinterpret_cast<int*>(&pctx.width), reinterpret_cast<int*>(&pctx.height));
	glfwGetWindowPos(pctx.window, reinterpret_cast<int*>(&pctx.x), reinterpret_cast<int*>(&pctx.y));
	if (pctx.window == nullptr) {
		glfwTerminate();
		throw std::runtime_error("Could not init window");
	}
#if defined(AKA_USE_OPENGL)
	glfwMakeContextCurrent(pctx.window);
	glfwSwapInterval(1); // default enable vsync
#endif

	// --- Callbacks ---
	// --- Size
	glfwSetWindowSizeCallback(pctx.window, [](GLFWwindow* window, int width, int height) {
		pctx.width = width;
		pctx.height = height;
		EventDispatcher<WindowResizeEvent>::emit(WindowResizeEvent{ (uint32_t)width, (uint32_t)height });
	});
	glfwSetFramebufferSizeCallback(pctx.window, [](GLFWwindow* window, int width, int height) {
		EventDispatcher<BackbufferResizeEvent>::emit(BackbufferResizeEvent{ (uint32_t)width, (uint32_t)height });
	});
	glfwSetWindowContentScaleCallback(pctx.window, [](GLFWwindow* window, float x, float y) {
		EventDispatcher<WindowContentScaledEvent>::emit(WindowContentScaledEvent{ x, y });
	});
	glfwSetWindowMaximizeCallback(pctx.window, [](GLFWwindow* window, int maximized) {
		EventDispatcher<WindowMaximizedEvent>::emit(WindowMaximizedEvent{ (maximized == GLFW_TRUE) });
	});
	// --- Window
	glfwSetWindowFocusCallback(pctx.window, [](GLFWwindow* window, int focused) {
		EventDispatcher<WindowFocusedEvent>::emit(WindowFocusedEvent{ (focused == GLFW_TRUE) });
	});
	glfwSetWindowRefreshCallback(pctx.window, [](GLFWwindow* window) {
		EventDispatcher<WindowRefreshedEvent>::emit(WindowRefreshedEvent{});
	});
	glfwSetWindowIconifyCallback(pctx.window, [](GLFWwindow* window, int iconified) {
		EventDispatcher<WindowIconifiedEvent>::emit(WindowIconifiedEvent{ (iconified == GLFW_TRUE) });
	});
	glfwSetWindowPosCallback(pctx.window, [](GLFWwindow* window, int x, int y) {
		EventDispatcher<WindowMovedEvent>::emit(WindowMovedEvent{ x, y });
		pctx.x = x;
		pctx.y = y;
	});
	// --- Monitor
	glfwSetMonitorCallback([](GLFWmonitor* monitor, int event) {
		// TODO get monitor informations
		if (event == GLFW_CONNECTED)
			EventDispatcher<MonitorConnectedEvent>::emit(MonitorConnectedEvent{});
		else if (event == GLFW_DISCONNECTED)
			EventDispatcher<MonitorDisconnectedEvent>::emit(MonitorDisconnectedEvent {});
	});
	// --- Inputs
	glfwSetKeyCallback(pctx.window, [](GLFWwindow* window, int key, int scancode, int action, int mode) {
		if (action == GLFW_PRESS)
			EventDispatcher<input::KeyboardKeyDownEvent>::emit(input::KeyboardKeyDownEvent{ glfwKeyMap[key] });
		else if (action == GLFW_RELEASE)
			EventDispatcher<input::KeyboardKeyUpEvent>::emit(input::KeyboardKeyUpEvent{ glfwKeyMap[key] });
		else if (action == GLFW_REPEAT)
			EventDispatcher<input::KeyboardKeyRepeatEvent>::emit(input::KeyboardKeyRepeatEvent{ glfwKeyMap[key] });
	});
	glfwSetMouseButtonCallback(pctx.window, [](GLFWwindow* window, int button, int action, int mode) {
		if (action == GLFW_PRESS)
			EventDispatcher<input::MouseButtonDownEvent>::emit(input::MouseButtonDownEvent{ static_cast<input::Button>(button) });
		else if (action == GLFW_RELEASE)
			EventDispatcher<input::MouseButtonUpEvent>::emit(input::MouseButtonUpEvent{ static_cast<input::Button>(button) });
		else if (action == GLFW_REPEAT)
			EventDispatcher<input::MouseButtonRepeatEvent>::emit(input::MouseButtonRepeatEvent{ static_cast<input::Button>(button) });
	});
	glfwSetCharCallback(pctx.window, [](GLFWwindow* window, unsigned int character) {
		EventDispatcher<WindowUnicodeCharEvent>::emit(WindowUnicodeCharEvent{ character });
	});
	glfwSetCursorPosCallback(pctx.window, [](GLFWwindow* window, double xpos, double ypos) {
		// position, in screen coordinates, relative to the upper-left corner of the client area of the window
		// Aka coordinates system origin is bottom left, so we convert.
		EventDispatcher<input::MouseMoveEvent>::emit(input::MouseMoveEvent{
			static_cast<float>(xpos), 
			static_cast<float>(pctx.height) - static_cast<float>(ypos) 
		});
	});
	glfwSetScrollCallback(pctx.window, [](GLFWwindow* window, double xoffset, double yoffset) {
		EventDispatcher<input::MouseScrollEvent>::emit(input::MouseScrollEvent{ 
			static_cast<float>(xoffset), 
			static_cast<float>(yoffset) 
		});
	});
	glfwSetCursorEnterCallback(pctx.window, [](GLFWwindow* window, int entered) {
		if (entered == GLFW_TRUE)
			EventDispatcher<input::MouseEnterEvent>::emit();
		else if (entered == GLFW_FALSE)
			EventDispatcher<input::MouseLeaveEvent>::emit();
	});
	glfwSetJoystickCallback([](int jid, int event) {
		if (event == GLFW_CONNECTED)
			EventDispatcher<input::JoystickConnectedEvent>::emit(input::JoystickConnectedEvent {jid});
		else if (event == GLFW_DISCONNECTED)
			EventDispatcher<input::JoystickDisconnectedEvent>::emit(input::JoystickDisconnectedEvent {jid});
	});
}

void PlatformBackend::destroy()
{
	glfwTerminate();
	pctx = {};
}

void PlatformBackend::update()
{
	glfwPollEvents();
	EventDispatcher<BackbufferResizeEvent>::dispatch();
	EventDispatcher<WindowResizeEvent>::dispatch();
	EventDispatcher<WindowMaximizedEvent>::dispatch();
	EventDispatcher<WindowContentScaledEvent>::dispatch();
	EventDispatcher<WindowIconifiedEvent>::dispatch();
	EventDispatcher<WindowFocusedEvent>::dispatch();
	EventDispatcher<WindowMovedEvent>::dispatch();
	EventDispatcher<WindowRefreshedEvent>::dispatch();
	EventDispatcher<WindowUnicodeCharEvent>::dispatch();
	EventDispatcher<MonitorConnectedEvent>::dispatch();
	EventDispatcher<MonitorDisconnectedEvent>::dispatch();
}

bool PlatformBackend::running()
{
	return !glfwWindowShouldClose(pctx.window);
}

void PlatformBackend::getSize(uint32_t* width, uint32_t* height)
{
	glfwGetWindowSize(pctx.window, reinterpret_cast<int*>(width), reinterpret_cast<int*>(height));
}

void PlatformBackend::setSize(uint32_t width, uint32_t height)
{
	glfwSetWindowSize(pctx.window, width, height);
}

void PlatformBackend::setLimits(uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight)
{
	auto convert = [](uint32_t value) -> int { if (value == 0) return GLFW_DONT_CARE; return value; };
	glfwSetWindowSizeLimits(pctx.window, convert(minWidth), convert(minHeight), convert(maxWidth), convert(maxHeight));
}

void PlatformBackend::setFullscreen(bool enabled)
{
	static uint32_t x = pctx.x;
	static uint32_t y = pctx.y;
	static uint32_t width = pctx.width;
	static uint32_t height = pctx.height;
	if (enabled)
	{
		if (glfwGetWindowMonitor(pctx.window) != nullptr)
			return; // Already full screen
		// Store windowed size for switching back from fullscreen
		x = pctx.x;
		y = pctx.y;
		width = pctx.width;
		height = pctx.height;
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(pctx.window, monitor, 0, 0, mode->width, mode->height, 0);
		// TODO correctly reenable vsync
		glfwSwapInterval(1);
	}
	else
	{
		if (glfwGetWindowMonitor(pctx.window) == nullptr)
			return; // Already not fullscreen
		glfwSetWindowMonitor(pctx.window, nullptr, x, y, width, height, GLFW_DONT_CARE);
	}
}

#if defined(AKA_PLATFORM_WINDOWS)
HWND PlatformBackend::getWindowsWindowHandle()
{
	return glfwGetWin32Window(pctx.window);
}
#endif

GLFWwindow* PlatformBackend::getGLFW3Handle()
{
	return pctx.window;
}

};
