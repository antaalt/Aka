#pragma once

#include "GraphicBackend.h"

#include "Platform.h"

namespace app {

struct OpenGLBackend : public GraphicBackend {
public:
	void initialize() override;
	void destroy() override;

	void clear() override;

	Texture* createTexture() override;
private:
	API m_api;
};

}
