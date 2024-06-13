#include "PlatformGLFW3.h"

#include <Aka/Platform/Input.h>
#include <Aka/Core/Event.h>
#include <Aka/Core/Application.h>
#include <Aka/OS/OS.h>
#include <Aka/OS/Logger.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
//#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

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



PlatformGLFW3::PlatformGLFW3(const PlatformConfig& config) :
	PlatformDevice(config),
	m_window(nullptr)
{
}

PlatformGLFW3::~PlatformGLFW3()
{
}

#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)
// Tag for memory tracking
struct GlfwAllocHandle {
	byte_t byte;
};
static_assert(sizeof(GlfwAllocHandle) == 1);
#endif

void PlatformGLFW3::initialize(const PlatformConfig& config)
{
	glfwSetErrorCallback([](int error, const char* description) {
		Logger::error("[GLFW][", error, "] ", description);
		});
#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)
	GLFWallocator allocator{};
	allocator.allocate = [](size_t size, void* user) -> void* {
		return mem::getAllocator(AllocatorMemoryType::Object, AllocatorCategory::Platform).allocate<GlfwAllocHandle>(size, AllocatorFlags::None);
	};
	allocator.deallocate = [](void* block, void* user) {
		mem::getAllocator(AllocatorMemoryType::Object, AllocatorCategory::Platform).deallocate(block);
	};
	allocator.reallocate = [](void* block, size_t size, void* user) -> void* {
		return mem::getAllocator(AllocatorMemoryType::Object, AllocatorCategory::Platform).reallocate<GlfwAllocHandle>(block, size, AllocatorFlags::None);
	};
	glfwInitAllocator(&allocator);
#endif
	// TODO glfw context static to handle multiple window creation
	if (glfwInit() != GLFW_TRUE)
		throw std::runtime_error("Could not init GLFW");
	auto has = [](PlatformFlag flags, PlatformFlag flag) -> int {
		return (flags & flag) == flag ? GLFW_TRUE : GLFW_FALSE;
		};
	// Backend API
	glfwWindowHint(GLFW_RESIZABLE, has(config.flags, PlatformFlag::Resizable));
	glfwWindowHint(GLFW_DECORATED, has(config.flags, PlatformFlag::Decorated));
	glfwWindowHint(GLFW_MAXIMIZED, has(config.flags, PlatformFlag::Maximized));
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, has(config.flags, PlatformFlag::Transparent));
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
#if defined(AKA_DEBUG)
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
#elif defined(AKA_USE_D3D11) || defined(AKA_USE_VULKAN)
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
	GLFWmonitor* monitor = nullptr;
	if ((config.flags & PlatformFlag::FullScreen) == PlatformFlag::FullScreen)
		monitor = glfwGetPrimaryMonitor();
	m_window = glfwCreateWindow(config.width, config.height, config.name.cstr(), monitor, nullptr);
	glfwGetWindowSize(m_window, reinterpret_cast<int*>(&m_width), reinterpret_cast<int*>(&m_height));
	glfwGetWindowPos(m_window, reinterpret_cast<int*>(&m_x), reinterpret_cast<int*>(&m_y));
	glfwSetWindowUserPointer(m_window, this);
	//glfwSetMonitorUserPointer(monitor, this);
	if (config.icon.size > 0)
	{
		GLFWimage img{ (int)config.icon.size, (int)config.icon.size, config.icon.bytes };
		glfwSetWindowIcon(m_window, 1, &img);
}
	if (m_window == nullptr) {
		glfwTerminate();
		throw std::runtime_error("Could not init window");
	}
#if defined(AKA_USE_OPENGL)
	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(1); // default enable vsync
#endif

	// --- Callbacks ---
	// --- Size
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		PlatformGLFW3* p = static_cast<PlatformGLFW3*>(glfwGetWindowUserPointer(window));
		p->m_width = width;
		p->m_height = height;
		EventDispatcher<WindowResizeEvent>::emit(WindowResizeEvent{ (uint32_t)width, (uint32_t)height });
		});
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		EventDispatcher<BackbufferResizeEvent>::emit(BackbufferResizeEvent{ (uint32_t)width, (uint32_t)height });
		});
	glfwSetWindowContentScaleCallback(m_window, [](GLFWwindow* window, float x, float y) {
		EventDispatcher<WindowContentScaledEvent>::emit(WindowContentScaledEvent{ x, y });
		});
	glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow* window, int maximized) {
		EventDispatcher<WindowMaximizedEvent>::emit(WindowMaximizedEvent{ (maximized == GLFW_TRUE) });
		});
	// --- Window
	glfwSetWindowFocusCallback(m_window, [](GLFWwindow* window, int focused) {
		EventDispatcher<WindowFocusedEvent>::emit(WindowFocusedEvent{ (focused == GLFW_TRUE) });
		});
	glfwSetWindowRefreshCallback(m_window, [](GLFWwindow* window) {
		EventDispatcher<WindowRefreshedEvent>::emit(WindowRefreshedEvent{});
		});
	glfwSetWindowIconifyCallback(m_window, [](GLFWwindow* window, int iconified) {
		EventDispatcher<WindowIconifiedEvent>::emit(WindowIconifiedEvent{ (iconified == GLFW_TRUE) });
		});
	glfwSetWindowPosCallback(m_window, [](GLFWwindow* window, int x, int y) {
		PlatformGLFW3* p = static_cast<PlatformGLFW3*>(glfwGetWindowUserPointer(window));
		EventDispatcher<WindowMovedEvent>::emit(WindowMovedEvent{ x, y });
		p->m_x = x;
		p->m_y = y;
		});
	// --- Monitor
	glfwSetMonitorCallback([](GLFWmonitor* monitor, int event) {
		// TODO get monitor informations
		if (event == GLFW_CONNECTED)
			EventDispatcher<MonitorConnectedEvent>::emit(MonitorConnectedEvent {});
		else if (event == GLFW_DISCONNECTED)
			EventDispatcher<MonitorDisconnectedEvent>::emit(MonitorDisconnectedEvent {});
		});
	// --- Inputs
	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mode) {
		PlatformGLFW3* p = static_cast<PlatformGLFW3*>(glfwGetWindowUserPointer(window));
		KeyboardKey k = glfwKeyboardKeyMap[key];
		if (action == GLFW_PRESS)
		{
			p->onKeyboardKeyDown(k);
			EventDispatcher<KeyboardKeyDownEvent>::emit(KeyboardKeyDownEvent{ k });
		}
		else if (action == GLFW_RELEASE)
		{
			p->onKeyboardKeyUp(k);
			EventDispatcher<KeyboardKeyUpEvent>::emit(KeyboardKeyUpEvent{ k });
		}
		else if (action == GLFW_REPEAT)
			EventDispatcher<KeyboardKeyRepeatEvent>::emit(KeyboardKeyRepeatEvent{ k });
		});
	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mode) {
		MouseButton b = glfwMouseButtonMap[button];
		PlatformGLFW3* p = static_cast<PlatformGLFW3*>(glfwGetWindowUserPointer(window));
		if (action == GLFW_PRESS)
		{
			p->onMouseButtonDown(b);
			EventDispatcher<MouseButtonDownEvent>::emit(MouseButtonDownEvent{ b });
		}
		else if (action == GLFW_RELEASE)
		{
			p->onMouseButtonUp(b);
			EventDispatcher<MouseButtonUpEvent>::emit(MouseButtonUpEvent{ b });
		}
		else if (action == GLFW_REPEAT)
			EventDispatcher<MouseButtonRepeatEvent>::emit(MouseButtonRepeatEvent{ b });
		});
	glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int character) {
		EventDispatcher<WindowUnicodeCharEvent>::emit(WindowUnicodeCharEvent{ character });
		});
	glfwSetDropCallback(m_window, [](GLFWwindow* window, int count, const char** paths) {
		// TODO get mouse position and pass it along the event.
		WindowDropEvent e;
		for (int i = 0; i < count; i++)
			e.paths.append(OS::normalize(paths[i]));
		EventDispatcher<WindowDropEvent>::emit(std::move(e));
		});
	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
		// position, in screen coordinates, relative to the upper-left corner of the client area of the window
		// Aka coordinates system origin is bottom left, so we convert.
		PlatformGLFW3* p = static_cast<PlatformGLFW3*>(glfwGetWindowUserPointer(window));
		float x = static_cast<float>(xpos);
		float y = static_cast<float>(p->height()) - static_cast<float>(ypos);
		p->onMouseMotion(x, y);
		EventDispatcher<MouseMotionEvent>::emit(MouseMotionEvent{ x, y });
		});
	glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
		PlatformGLFW3* p = static_cast<PlatformGLFW3*>(glfwGetWindowUserPointer(window));
		float x = static_cast<float>(xoffset);
		float y = static_cast<float>(yoffset);
		p->onMouseScroll(x, y);
		EventDispatcher<MouseScrollEvent>::emit(MouseScrollEvent{ x, y });
		});
	glfwSetCursorEnterCallback(m_window, [](GLFWwindow* window, int entered) {
		PlatformGLFW3* p = static_cast<PlatformGLFW3*>(glfwGetWindowUserPointer(window));
		if (entered == GLFW_TRUE)
		{
			p->onMouseEnter();
			EventDispatcher<MouseEnterEvent>::emit();
		}
		else if (entered == GLFW_FALSE)
		{
			p->onMouseLeave();
			EventDispatcher<MouseLeaveEvent>::emit();
		}
		});
	glfwSetJoystickCallback([](int jid, int event) {
		PlatformGLFW3* p = static_cast<PlatformGLFW3*>(Application::app()->platform());
		AKA_ASSERT(p != nullptr, "Invalid platform retrieved");
		if (glfwJoystickIsGamepad(jid) == GLFW_FALSE)
			return;
		GamepadID gid = (GamepadID)jid;
		if (event == GLFW_CONNECTED)
		{
			const char* name = glfwGetGamepadName(jid);
			p->onGamepadConnected(gid, name);
			EventDispatcher<GamepadConnectedEvent>::emit(GamepadConnectedEvent{ gid, name });
		}
		else if (event == GLFW_DISCONNECTED)
		{
			p->onGamepadDisconnected(gid);
			EventDispatcher<GamepadDisconnectedEvent>::emit(GamepadDisconnectedEvent{ gid });
		}
		});
	// Register all connected joystick by emitting a connected event
	for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; jid++)
	{
		GamepadID gid = (GamepadID)jid;
		if (glfwJoystickPresent(jid) == GLFW_TRUE && glfwJoystickIsGamepad(jid) == GLFW_TRUE)
		{
			const char* name = glfwGetGamepadName(jid);
			onGamepadConnected(gid, name);
			EventDispatcher<GamepadConnectedEvent>::emit(GamepadConnectedEvent{ gid, name });
		}
	}
}

void PlatformGLFW3::shutdown()
{
	glfwTerminate();
}

void PlatformGLFW3::poll()
{
	onInputsUpdate();
	glfwPollEvents();
	// Generate joystick events as GLFW does not seem to have built in way...
	for (int jid = GLFW_JOYSTICK_1; jid < GLFW_JOYSTICK_LAST; ++jid)
	{
		GamepadID gid = static_cast<GamepadID>(jid);
		auto it = m_gamepads.find(gid);
		if (it == m_gamepads.end())
			continue;
		Gamepad& gamepad = it->second;
		GLFWgamepadstate state;
		if (glfwGetGamepadState(jid, &state) == GLFW_TRUE)
		{
			for (int iButton = 0; iButton < sizeof(state.buttons) / sizeof(state.buttons[0]); iButton++)
			{ 
				if (state.buttons[iButton] == GLFW_PRESS && !gamepad.pressed(gid, glfwGamepadButtonMap[iButton]))
					EventDispatcher<GamepadButtonDownEvent>::emit(GamepadButtonDownEvent{ gid, glfwGamepadButtonMap[iButton] });
				else if (state.buttons[iButton] == GLFW_RELEASE && gamepad.pressed(gid, glfwGamepadButtonMap[iButton]))
					EventDispatcher<GamepadButtonUpEvent>::emit(GamepadButtonUpEvent{ gid, glfwGamepadButtonMap[iButton] });
			}
			const Position& leftAxis = gamepad.axis((GamepadID)gid, GamepadAxis::Left);
			if (leftAxis.x != state.axes[GLFW_GAMEPAD_AXIS_LEFT_X] || leftAxis.y != state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y])
			{
				EventDispatcher<GamepadAxesMotionEvent>::emit(GamepadAxesMotionEvent{
					gid,
					GamepadAxis::Left,
					Position{ state.axes[GLFW_GAMEPAD_AXIS_LEFT_X], state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] }
				});
			}
			const Position& rightAxis = gamepad.axis((GamepadID)gid, GamepadAxis::Right);
			if (rightAxis.x != state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] || rightAxis.y != state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y])
			{
				EventDispatcher<GamepadAxesMotionEvent>::emit(GamepadAxesMotionEvent{
					gid,
					GamepadAxis::Right,
					Position{ state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X], state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] }
				});
			}
			const Position& triggerLeftAxis = gamepad.axis((GamepadID)gid, GamepadAxis::TriggerLeft);
			if (triggerLeftAxis.x != state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER])
			{
				EventDispatcher<GamepadAxesMotionEvent>::emit(GamepadAxesMotionEvent{
					gid,
					GamepadAxis::TriggerLeft,
					Position{ state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER], 0.f }
				});
			}
			const Position& triggerRightAxis = gamepad.axis((GamepadID)gid, GamepadAxis::TriggerRight);
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

	if (glfwWindowShouldClose(m_window))
		EventDispatcher<QuitEvent>::emit();

	// Dispatch all input events
	EventDispatcher<KeyboardKeyDownEvent>::dispatch();
	EventDispatcher<KeyboardKeyUpEvent>::dispatch();
	EventDispatcher<MouseButtonDownEvent>::dispatch();
	EventDispatcher<MouseButtonUpEvent>::dispatch();
	EventDispatcher<MouseMotionEvent>::dispatch();
	EventDispatcher<MouseScrollEvent>::dispatch();
	EventDispatcher<MouseEnterEvent>::dispatch();
	EventDispatcher<MouseLeaveEvent>::dispatch();
	EventDispatcher<GamepadConnectedEvent>::dispatch();
	EventDispatcher<GamepadDisconnectedEvent>::dispatch();
	EventDispatcher<GamepadButtonDownEvent>::dispatch();
	EventDispatcher<GamepadButtonUpEvent>::dispatch();
	EventDispatcher<GamepadAxesMotionEvent>::dispatch();
}

void PlatformGLFW3::move(int32_t x, int32_t y)
{
	glfwSetWindowPos(m_window, x, y);
}

void PlatformGLFW3::resize(uint32_t width, uint32_t height)
{
	if ((m_flags & PlatformFlag::Resizable) != PlatformFlag::Resizable)
		return;
	glfwSetWindowSize(m_window, width, height);
}

void PlatformGLFW3::setLimits(uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight)
{
	auto convert = [](uint32_t value) -> int { if (value == 0) return GLFW_DONT_CARE; return value; };
	glfwSetWindowSizeLimits(m_window, convert(minWidth), convert(minHeight), convert(maxWidth), convert(maxHeight));
}

void PlatformGLFW3::fullscreen(bool enable)
{
	// TODO do we really need to store this ? do not make it static
	static uint32_t x = m_x;
	static uint32_t y = m_y;
	static uint32_t width = m_width;
	static uint32_t height = m_height;
	// Check current fullscreen state.
	bool enabled = (m_flags & PlatformFlag::FullScreen) == PlatformFlag::FullScreen;
	if (enabled == enable)
		return;
	if (enable)
	{
		AKA_ASSERT(glfwGetWindowMonitor(m_window) == nullptr, "Fullscreen should not be enabled.");
		m_flags = m_flags | PlatformFlag::FullScreen;
		// Store windowed size for switching back from fullscreen
		x = m_x;
		y = m_y;
		width = m_width;
		height = m_height;
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, 0);
		// TODO correctly reenable vsync. This should be swapchain dependent ?
		glfwSwapInterval(1);
	}
	else
	{
		AKA_ASSERT(glfwGetWindowMonitor(m_window) != nullptr, "Fullscreen should be enabled.");
		m_flags = m_flags & ~PlatformFlag::FullScreen;
		glfwSetWindowMonitor(m_window, nullptr, x, y, width, height, GLFW_DONT_CARE);
	}
}

GLFWwindow* PlatformGLFW3::getGLFW3Handle()
{
	return m_window;
}

void* PlatformGLFW3::getNativeHandle()
{
	return glfwGetWin32Window(m_window);
}

};