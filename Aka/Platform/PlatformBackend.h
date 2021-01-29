#pragma once

#include <stdint.h>

#include "Platform.h"
#include "../OS/FileSystem.h"

namespace aka {

struct Config;

class PlatformBackend
{
public:
	static void initialize(const Config& config);
	static void destroy();
	static void frame();
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
	static std::vector<Path> enumerate(const Path& path);
	static Path normalize(const Path& path);
	static Path executablePath();
	static Path cwd();
	static std::string extension(const Path& path);
	static std::string fileName(const Path& path);

	static GLFWwindow* getGLFW3Handle();

#if defined(AKA_USE_D3D11)
	static HWND getD3DHandle();
#endif
};

};