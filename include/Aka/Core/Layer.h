#pragma once

#include <Aka/OS/Time.h>
#include <Aka/Core/Application.h>

namespace aka {

class Layer :
	EventListener<AppCreateEvent>,
	EventListener<AppDestroyEvent>,
	EventListener<AppFixedUpdateEvent>,
	EventListener<AppUpdateEvent>,
	EventListener<AppRenderEvent>,
	EventListener<AppFrameEvent>,
	EventListener<AppPresentEvent>,
	EventListener<AppResizeEvent>
{
public:
	Layer() {}
	virtual ~Layer() {}
private:
	void onReceive(const AppCreateEvent& event) { onLayerCreate(); }
	void onReceive(const AppDestroyEvent& event) { onLayerDestroy(); }
	void onReceive(const AppFixedUpdateEvent& event) { onLayerFixedUpdate(event.deltaTime); }
	void onReceive(const AppUpdateEvent& event) { onLayerUpdate(event.deltaTime); }
	void onReceive(const AppRenderEvent& event) { onLayerRender(event.frame); }
	void onReceive(const AppFrameEvent& event) { onLayerFrame(); }
	void onReceive(const AppPresentEvent& event) { onLayerPresent(); }
	void onReceive(const AppResizeEvent& event) { onLayerResize(event.width, event.height); }
protected:
	virtual void onLayerCreate() {}
	virtual void onLayerDestroy() {}

	virtual void onLayerUpdate(Time deltaTime) {}
	virtual void onLayerFixedUpdate(Time deltaTime) {}
	virtual void onLayerFrame() {}
	virtual void onLayerRender(Frame* frame) {}
	virtual void onLayerPresent() {}

	virtual void onLayerResize(uint32_t width, uint32_t height) {}
};

};