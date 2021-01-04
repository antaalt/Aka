#pragma once

#include "Texture.h"
//#include "Framebuffer.h"

namespace app {


struct GraphicBackend {
	enum class API {
		OPENGL,
		DIRECTX
	};
	struct Config {
		API api;
	};

	API api() const { m_api; }

public:
	virtual void initialize() = 0;
	virtual void destroy() = 0;

	virtual void clear() = 0;

	virtual Texture* createTexture() = 0;
private:
	API m_api;
};

}