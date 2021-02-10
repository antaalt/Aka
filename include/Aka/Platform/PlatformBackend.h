#pragma once

#include "Platform.h"
#include "../OS/FileSystem.h"

#include <stdint.h>

struct GLFWwindow;
#if defined(AKA_PLATFORM_WINDOWS)
struct HWND__;
typedef HWND__* HWND;
#endif

namespace aka {

struct Config;

class PlatformBackend
{
public:
	static void initialize(const Config& config);
	static void destroy();
	static void update();
	static bool running();
	static void getSize(uint32_t* width, uint32_t* height);
	static void setSize(uint32_t width, uint32_t height);
	static void setLimits(uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight);
	static bool directoryExist(const Path &path);
	static bool directoryCreate(const Path& path);
	static bool directoryRemove(const Path& path, bool recursive = false);
	static bool fileExist(const Path& path);
	static bool fileCreate(const Path& path);
	static bool fileRemove(const Path& path);
	static bool loadString(const Path& path, std::string* bytes);
	static bool loadBinary(const Path& path, std::vector<uint8_t>* str);
	static bool writeString(const Path& path, const std::string& bytes);
	static bool writeBinary(const Path& path, const std::vector<uint8_t>& str);
	static std::vector<Path> enumerate(const Path& path);
	static Path normalize(const Path& path);
	static Path executablePath();
	static Path cwd();
	static std::string extension(const Path& path);
	static std::string fileName(const Path& path);
	static void errorDialog(const std::string& message);

#if defined(AKA_PLATFORM_WINDOWS)
	static HWND getWindowsWindowHandle();
#endif
	static GLFWwindow* getGLFW3Handle();
};

};