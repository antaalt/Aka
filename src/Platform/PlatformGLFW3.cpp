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

const KeyboardKey glfwKeyboardKeyMap[512] = {
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Space, // GLFW_KEY_SPACE              32
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Apostrophe, // GLFW_KEY_APOSTROPHE         39
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Comma, // GLFW_KEY_COMMA              44
	KeyboardKey::Minus, // GLFW_KEY_MINUS              45
	KeyboardKey::Period, // GLFW_KEY_PERIOD             46
	KeyboardKey::Slash, // GLFW_KEY_SLASH              47
	KeyboardKey::Num0, // GLFW_KEY_0                  48
	KeyboardKey::Num1, // GLFW_KEY_1                  49
	KeyboardKey::Num2, // GLFW_KEY_2                  50
	KeyboardKey::Num3, // GLFW_KEY_3                  51
	KeyboardKey::Num4, // GLFW_KEY_4                  52
	KeyboardKey::Num5, // GLFW_KEY_5                  53
	KeyboardKey::Num6, // GLFW_KEY_6                  54
	KeyboardKey::Num7, // GLFW_KEY_7                  55
	KeyboardKey::Num8, // GLFW_KEY_8                  56
	KeyboardKey::Num9, // GLFW_KEY_9                  57
	KeyboardKey::Unknown,
	KeyboardKey::Semicolon, // GLFW_KEY_SEMICOLON          59
	KeyboardKey::Unknown,
	KeyboardKey::Equal, // GLFW_KEY_EQUAL              61
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::A, // GLFW_KEY_A                  65
	KeyboardKey::B, // GLFW_KEY_B                  66
	KeyboardKey::C, // GLFW_KEY_C                  67
	KeyboardKey::D, // GLFW_KEY_D                  68
	KeyboardKey::E, // GLFW_KEY_E                  69
	KeyboardKey::F, // GLFW_KEY_F                  70
	KeyboardKey::G, // GLFW_KEY_G                  71
	KeyboardKey::H, // GLFW_KEY_H                  72
	KeyboardKey::I, // GLFW_KEY_I                  73
	KeyboardKey::J, // GLFW_KEY_J                  74
	KeyboardKey::K, // GLFW_KEY_K                  75
	KeyboardKey::L, // GLFW_KEY_L                  76
	KeyboardKey::M, // GLFW_KEY_M                  77
	KeyboardKey::N, // GLFW_KEY_N                  78
	KeyboardKey::O, // GLFW_KEY_O                  79
	KeyboardKey::P, // GLFW_KEY_P                  80
	KeyboardKey::Q, // GLFW_KEY_Q                  81
	KeyboardKey::R, // GLFW_KEY_R                  82
	KeyboardKey::S, // GLFW_KEY_S                  83
	KeyboardKey::T, // GLFW_KEY_T                  84
	KeyboardKey::U, // GLFW_KEY_U                  85
	KeyboardKey::V, // GLFW_KEY_V                  86
	KeyboardKey::W, // GLFW_KEY_W                  87
	KeyboardKey::X, // GLFW_KEY_X                  88
	KeyboardKey::Y, // GLFW_KEY_Y                  89
	KeyboardKey::Z, // GLFW_KEY_Z                  90
	KeyboardKey::BracketLeft, // GLFW_KEY_LEFT_BRACKET       91
	KeyboardKey::BackSlash, // GLFW_KEY_BACKSLASH          92
	KeyboardKey::BracketRight, // GLFW_KEY_RIGHT_BRACKET      93
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Grave, // GLFW_KEY_GRAVE_ACCENT       96
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown, // GLFW_KEY_WORLD_1            161
	KeyboardKey::Unknown, // GLFW_KEY_WORLD_2            162
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Escape, // GLFW_KEY_ESCAPE             256
	KeyboardKey::Enter, // GLFW_KEY_ENTER              257
	KeyboardKey::Tab, // GLFW_KEY_TAB                258
	KeyboardKey::BackSpace, // GLFW_KEY_BACKSPACE          259
	KeyboardKey::Insert, // GLFW_KEY_INSERT             260
	KeyboardKey::Delete, // GLFW_KEY_DELETE             261
	KeyboardKey::ArrowRight, // GLFW_KEY_RIGHT              262
	KeyboardKey::ArrowLeft, // GLFW_KEY_LEFT               263
	KeyboardKey::ArrowDown, // GLFW_KEY_DOWN               264
	KeyboardKey::ArrowUp, // GLFW_KEY_UP                 265
	KeyboardKey::PageUp, // GLFW_KEY_PAGE_UP            266
	KeyboardKey::PageDown, // GLFW_KEY_PAGE_DOWN          267
	KeyboardKey::Home, // GLFW_KEY_HOME               268
	KeyboardKey::End, // GLFW_KEY_END                269
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::CapsLock, // GLFW_KEY_CAPS_LOCK          280
	KeyboardKey::ScrollLock, // GLFW_KEY_SCROLL_LOCK        281
	KeyboardKey::NumLock, // GLFW_KEY_NUM_LOCK           282
	KeyboardKey::PrintScreen, // GLFW_KEY_PRINT_SCREEN       283
	KeyboardKey::Pause, // GLFW_KEY_PAUSE              284
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::F1, // GLFW_KEY_F1                 290
	KeyboardKey::F2, // GLFW_KEY_F2                 291
	KeyboardKey::F3, // GLFW_KEY_F3                 292
	KeyboardKey::F4, // GLFW_KEY_F4                 293
	KeyboardKey::F5, // GLFW_KEY_F5                 294
	KeyboardKey::F6, // GLFW_KEY_F6                 295
	KeyboardKey::F7, // GLFW_KEY_F7                 296
	KeyboardKey::F8, // GLFW_KEY_F8                 297
	KeyboardKey::F9, // GLFW_KEY_F9                 298
	KeyboardKey::F10, // GLFW_KEY_F10                299
	KeyboardKey::F11, // GLFW_KEY_F11                300
	KeyboardKey::F12, // GLFW_KEY_F12                301
	KeyboardKey::Unknown, // GLFW_KEY_F13                302
	KeyboardKey::Unknown, // GLFW_KEY_F14                303
	KeyboardKey::Unknown, // GLFW_KEY_F15                304
	KeyboardKey::Unknown, // GLFW_KEY_F16                305
	KeyboardKey::Unknown, // GLFW_KEY_F17                306
	KeyboardKey::Unknown, // GLFW_KEY_F18                307
	KeyboardKey::Unknown, // GLFW_KEY_F19                308
	KeyboardKey::Unknown, // GLFW_KEY_F20                309
	KeyboardKey::Unknown, // GLFW_KEY_F21                310
	KeyboardKey::Unknown, // GLFW_KEY_F22                311
	KeyboardKey::Unknown, // GLFW_KEY_F23                312
	KeyboardKey::Unknown, // GLFW_KEY_F24                313
	KeyboardKey::Unknown, // GLFW_KEY_F25                314
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::KeyPad0, // GLFW_KEY_KP_0               320
	KeyboardKey::KeyPad1, // GLFW_KEY_KP_1               321
	KeyboardKey::KeyPad2, // GLFW_KEY_KP_2               322
	KeyboardKey::KeyPad3, // GLFW_KEY_KP_3               323
	KeyboardKey::KeyPad4, // GLFW_KEY_KP_4               324
	KeyboardKey::KeyPad5, // GLFW_KEY_KP_5               325
	KeyboardKey::KeyPad6, // GLFW_KEY_KP_6               326
	KeyboardKey::KeyPad7, // GLFW_KEY_KP_7               327
	KeyboardKey::KeyPad8, // GLFW_KEY_KP_8               328
	KeyboardKey::KeyPad9, // GLFW_KEY_KP_9               329
	KeyboardKey::KeyPadDecimal, // GLFW_KEY_KP_DECIMAL         330
	KeyboardKey::KeyPadDivide, // GLFW_KEY_KP_DIVIDE          331
	KeyboardKey::KeyPadMultiply, // GLFW_KEY_KP_MULTIPLY        332
	KeyboardKey::KeyPadSubstract, // GLFW_KEY_KP_SUBTRACT        333
	KeyboardKey::KeyPadAdd, // GLFW_KEY_KP_ADD             334
	KeyboardKey::KeyPadEnter, // GLFW_KEY_KP_ENTER           335
	KeyboardKey::Unknown, // GLFW_KEY_KP_EQUAL           336
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::Unknown,
	KeyboardKey::ShiftLeft, // GLFW_KEY_LEFT_SHIFT         340
	KeyboardKey::ControlLeft, // GLFW_KEY_LEFT_CONTROL       341
	KeyboardKey::AltLeft, // GLFW_KEY_LEFT_ALT           342
	KeyboardKey::Unknown, // GLFW_KEY_LEFT_SUPER         343
	KeyboardKey::ShiftRight, // GLFW_KEY_RIGHT_SHIFT        344
	KeyboardKey::ControlRight, // GLFW_KEY_RIGHT_CONTROL      345
	KeyboardKey::AltRight, // GLFW_KEY_RIGHT_ALT          346
	KeyboardKey::Unknown, // GLFW_KEY_RIGHT_SUPER        347
	KeyboardKey::Menu, // GLFW_KEY_MENU               348
};

const MouseButton glfwMouseButtonMap[8] = {
	MouseButton::Button1, // GLFW_MOUSE_BUTTON_1
	MouseButton::Button2, // GLFW_MOUSE_BUTTON_1
	MouseButton::Button3, // GLFW_MOUSE_BUTTON_1
	MouseButton::Button4, // GLFW_MOUSE_BUTTON_1
	MouseButton::Button5, // GLFW_MOUSE_BUTTON_1
	MouseButton::Button6, // GLFW_MOUSE_BUTTON_1
	MouseButton::Button7, // GLFW_MOUSE_BUTTON_1
	MouseButton::Button8, // GLFW_MOUSE_BUTTON_1
};

const GamepadButton glfwGamepadButtonMap[512] = {
	GamepadButton::A, // GLFW_GAMEPAD_BUTTON_A
	GamepadButton::B, // GLFW_GAMEPAD_BUTTON_B
	GamepadButton::X, // GLFW_GAMEPAD_BUTTON_X
	GamepadButton::Y, // GLFW_GAMEPAD_BUTTON_Y
	GamepadButton::LeftBumper, // GLFW_GAMEPAD_BUTTON_LEFT_BUMPER
	GamepadButton::RightBumper, // GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER
	GamepadButton::Select, // GLFW_GAMEPAD_BUTTON_BACK
	GamepadButton::Start, // GLFW_GAMEPAD_BUTTON_START
	GamepadButton::Home, // GLFW_GAMEPAD_BUTTON_GUIDE
	GamepadButton::LeftStick, // GLFW_GAMEPAD_BUTTON_LEFT_THUMB
	GamepadButton::RightStick, // GLFW_GAMEPAD_BUTTON_RIGHT_THUMB
	GamepadButton::DpadUp, // GLFW_GAMEPAD_BUTTON_DPAD_UP
	GamepadButton::DpadRight, // GLFW_GAMEPAD_BUTTON_DPAD_RIGHT
	GamepadButton::DpadDown, // GLFW_GAMEPAD_BUTTON_DPAD_DOWN
	GamepadButton::DpadLeft, // GLFW_GAMEPAD_BUTTON_DPAD_LEFT
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

	pctx.window = glfwCreateWindow(config.width, config.height, config.name.cstr(), NULL, NULL);
	glfwGetWindowSize(pctx.window, reinterpret_cast<int*>(&pctx.width), reinterpret_cast<int*>(&pctx.height));
	glfwGetWindowPos(pctx.window, reinterpret_cast<int*>(&pctx.x), reinterpret_cast<int*>(&pctx.y));
	if (config.icon.size() > 0)
	{
		Image image = config.icon;
		GLFWimage img{ (int)config.icon.width(), (int)config.icon.height(), static_cast<unsigned char*>(image.data()) };
		glfwSetWindowIcon(pctx.window, 1, &img);
	}
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
			EventDispatcher<KeyboardKeyDownEvent>::emit(KeyboardKeyDownEvent{ glfwKeyboardKeyMap[key] });
		else if (action == GLFW_RELEASE)
			EventDispatcher<KeyboardKeyUpEvent>::emit(KeyboardKeyUpEvent{ glfwKeyboardKeyMap[key] });
		else if (action == GLFW_REPEAT)
			EventDispatcher<KeyboardKeyRepeatEvent>::emit(KeyboardKeyRepeatEvent{ glfwKeyboardKeyMap[key] });
	});
	glfwSetMouseButtonCallback(pctx.window, [](GLFWwindow* window, int button, int action, int mode) {
		if (action == GLFW_PRESS)
			EventDispatcher<MouseButtonDownEvent>::emit(MouseButtonDownEvent{ glfwMouseButtonMap[button] });
		else if (action == GLFW_RELEASE)
			EventDispatcher<MouseButtonUpEvent>::emit(MouseButtonUpEvent{ glfwMouseButtonMap[button] });
		else if (action == GLFW_REPEAT)
			EventDispatcher<MouseButtonRepeatEvent>::emit(MouseButtonRepeatEvent{ glfwMouseButtonMap[button] });
	});
	glfwSetCharCallback(pctx.window, [](GLFWwindow* window, unsigned int character) {
		EventDispatcher<WindowUnicodeCharEvent>::emit(WindowUnicodeCharEvent{ character });
	});
	glfwSetDropCallback(pctx.window, [](GLFWwindow* window, int count, const char** paths) {
		WindowDropEvent e;
		for (int i = 0; i < count; i++)
			e.paths.append(Path::normalize(paths[i]));
		EventDispatcher<WindowDropEvent>::emit(std::move(e));
	});
	glfwSetCursorPosCallback(pctx.window, [](GLFWwindow* window, double xpos, double ypos) {
		// position, in screen coordinates, relative to the upper-left corner of the client area of the window
		// Aka coordinates system origin is bottom left, so we convert.
		EventDispatcher<MouseMoveEvent>::emit(MouseMoveEvent{
			static_cast<float>(xpos), 
			static_cast<float>(pctx.height) - static_cast<float>(ypos) 
		});
	});
	glfwSetScrollCallback(pctx.window, [](GLFWwindow* window, double xoffset, double yoffset) {
		EventDispatcher<MouseScrollEvent>::emit(MouseScrollEvent{ 
			static_cast<float>(xoffset), 
			static_cast<float>(yoffset) 
		});
	});
	glfwSetCursorEnterCallback(pctx.window, [](GLFWwindow* window, int entered) {
		if (entered == GLFW_TRUE)
			EventDispatcher<MouseEnterEvent>::emit();
		else if (entered == GLFW_FALSE)
			EventDispatcher<MouseLeaveEvent>::emit();
	});
	glfwSetJoystickCallback([](int jid, int event) {
		if (glfwJoystickIsGamepad(jid) == GLFW_FALSE)
			return;
		GamepadID gid = (GamepadID)jid;
		if (event == GLFW_CONNECTED)
			EventDispatcher<GamepadConnectedEvent>::emit(GamepadConnectedEvent {gid, glfwGetGamepadName(jid) });
		else if (event == GLFW_DISCONNECTED)
			EventDispatcher<GamepadDisconnectedEvent>::emit(GamepadDisconnectedEvent {gid});
	});
	// Register all connected joystick by emitting a connected event
	for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; jid++)
	{
		if (glfwJoystickPresent(jid) == GLFW_TRUE && glfwJoystickIsGamepad(jid) == GLFW_TRUE)
			EventDispatcher<GamepadConnectedEvent>::emit(GamepadConnectedEvent{ (GamepadID)jid, glfwGetGamepadName(jid) });
	}
}

void PlatformBackend::destroy()
{
	glfwTerminate();
	pctx = {};
}

void PlatformBackend::update()
{
	glfwPollEvents();
	// Generate joystick events as GLFW does not seem to have built in way...
	for (int jid = GLFW_JOYSTICK_1; jid < GLFW_JOYSTICK_LAST; ++jid)
	{
		GamepadID gid = static_cast<GamepadID>(jid);
		if (!Gamepad::connected(gid))
			continue;
		GLFWgamepadstate state;
		if (glfwGetGamepadState(jid, &state) == GLFW_TRUE)
		{
			for (int iButton = 0; iButton < sizeof(state.buttons) / sizeof(state.buttons[0]); iButton++)
			{ 
				if (state.buttons[iButton] == GLFW_PRESS && !Gamepad::pressed(gid, glfwGamepadButtonMap[iButton]))
					EventDispatcher<GamepadButtonDownEvent>::emit(GamepadButtonDownEvent{ gid, glfwGamepadButtonMap[iButton] });
				else if (state.buttons[iButton] == GLFW_RELEASE && Gamepad::pressed(gid, glfwGamepadButtonMap[iButton]))
					EventDispatcher<GamepadButtonUpEvent>::emit(GamepadButtonUpEvent{ gid, glfwGamepadButtonMap[iButton] });
			}
			const Position& leftAxis = Gamepad::axis((GamepadID)gid, GamepadAxis::Left);
			if (leftAxis.x != state.axes[GLFW_GAMEPAD_AXIS_LEFT_X] || leftAxis.y != state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y])
			{
				EventDispatcher<GamepadAxesMotionEvent>::emit(GamepadAxesMotionEvent{
					gid,
					GamepadAxis::Left,
					Position{ state.axes[GLFW_GAMEPAD_AXIS_LEFT_X], state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] }
				});
			}
			const Position& rightAxis = Gamepad::axis((GamepadID)gid, GamepadAxis::Right);
			if (rightAxis.x != state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] || rightAxis.y != state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y])
			{
				EventDispatcher<GamepadAxesMotionEvent>::emit(GamepadAxesMotionEvent{
					gid,
					GamepadAxis::Right,
					Position{ state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X], state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] }
				});
			}
			const Position& triggerLeftAxis = Gamepad::axis((GamepadID)gid, GamepadAxis::TriggerLeft);
			if (triggerLeftAxis.x != state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER])
			{
				EventDispatcher<GamepadAxesMotionEvent>::emit(GamepadAxesMotionEvent{
					gid,
					GamepadAxis::TriggerLeft,
					Position{ state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER], 0.f }
				});
			}
			const Position& triggerRightAxis = Gamepad::axis((GamepadID)gid, GamepadAxis::TriggerRight);
			if (triggerRightAxis.x != state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER])
			{
				EventDispatcher<GamepadAxesMotionEvent>::emit(GamepadAxesMotionEvent{
					gid,
					GamepadAxis::TriggerRight,
					Position{ state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER], 0.f }
				});
			}
		}
	}
	EventDispatcher<BackbufferResizeEvent>::dispatch();
	EventDispatcher<WindowResizeEvent>::dispatch();
	EventDispatcher<WindowMaximizedEvent>::dispatch();
	EventDispatcher<WindowContentScaledEvent>::dispatch();
	EventDispatcher<WindowIconifiedEvent>::dispatch();
	EventDispatcher<WindowFocusedEvent>::dispatch();
	EventDispatcher<WindowMovedEvent>::dispatch();
	EventDispatcher<WindowRefreshedEvent>::dispatch();
	EventDispatcher<WindowUnicodeCharEvent>::dispatch();
	EventDispatcher<WindowDropEvent>::dispatch();
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
