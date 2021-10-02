#pragma once

#include <Aka/OS/Time.h>

namespace aka {

class Layer
{
public:
	Layer() {}
	virtual ~Layer() {}

	virtual void onLayerAttach() {}
	virtual void onLayerDetach() {}

	virtual void onLayerUpdate(Time deltaTime) {}
	virtual void onLayerFixedUpdate(Time deltaTime) {}
	virtual void onLayerFrame() {}
	virtual void onLayerRender() {}
	virtual void onLayerPresent() {}

	virtual void onLayerResize(uint32_t width, uint32_t height) {}
};

};