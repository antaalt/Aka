#pragma once 

#include <memory>

#include <Aka/OS/Time.h>

namespace aka {

// A view (aka. a scene) is a part of the game.
class View
{
public:
	using Ptr = std::shared_ptr<View>;

	template <typename T, typename... Args>
	static typename T::Ptr create(Args&&... args);
public:
	View() {}
	virtual ~View() {}
	// Called on view creation
	virtual void onCreate() {}
	// Called on view destruction
	virtual void onDestroy() {}
	// Called on every view update
	virtual void onUpdate(Time deltaTime) {}
	// Called on every view update at a fixed timestep
	virtual void onFixedUpdate(Time deltaTime) {}
	// Called on view frame beginning
	virtual void onFrame() {}
	// Called on view frame render
	virtual void onRender() {}
	// Called on view frame end
	virtual void onPresent() {}
	// Called on view resize
	virtual void onResize(uint32_t width, uint32_t height) {}
};

template <typename T, typename... Args>
typename T::Ptr View::create(Args&&... args) {
	static_assert(std::is_base_of<View, T>::value, "Type is not a view");
	return std::make_unique<T>(std::forward<Args>(args)...);
}

};