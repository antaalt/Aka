#pragma once 

#include <memory>

#include <Aka/OS/Time.h>

namespace aka {

class Router;

enum class ViewID : uint32_t {};

ViewID generate();

// A view (aka. a scene) is a part of the game.
class View {
public:
	using Ptr = std::unique_ptr<View>;
public:
	View() {}
	virtual ~View() {}
	// Called on view creation
	virtual void onCreate() {}
	// Called on view destruction
	virtual void onDestroy() {}
	// Called on every view update
	virtual void onUpdate(Router& router, Time::Unit deltaTime) {}
	// Called on view frame beginning
	virtual void onFrame() {}
	// Called on view frame render
	virtual void onRender() {}
	// Called on view frame end
	virtual void onPresent() {}
	// Called on view resize
	virtual void onResize(uint32_t width, uint32_t height) {}
};

};