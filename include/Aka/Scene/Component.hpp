#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

class AssetLibrary;
class Renderer;

enum class ComponentID : uint8_t { Invalid = (uint8_t)-1 };

enum class ComponentState
{
	PendingActivation,
	Active,
	PendingDestruction,
};

class Component
{
public:
	Component(ComponentID id);
	virtual ~Component() {}

protected:
	friend class Node;
	void attach();
	void detach();
	void activate(AssetLibrary* library, Renderer* _renderer);
	void deactivate(AssetLibrary* library, Renderer* _renderer);
	void update(Time deltaTime);
	void fixedUpdate(Time deltaTime);
	void renderUpdate(AssetLibrary* library, Renderer* _renderer);
protected:
	// When a component just got attached, called just after constructor
	virtual void onAttach() {}
	// When a component just got detached, called just before destructor
	virtual void onDetach() {}
	// When a component become active.
	virtual void onBecomeActive(AssetLibrary* library, Renderer* _renderer) {}
	// When a component become inactive.
	virtual void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) {}

	// When we update the component
	virtual void onUpdate(Time deltaTime) {}
	// When we update the component with fixed time step.
	virtual void onFixedUpdate(Time deltaTime) {}
	// When update to renderer are required.
	// Should not be called every frame ?
	virtual void onRenderUpdate(AssetLibrary* library, Renderer* _renderer) {}
protected:
	template <typename T>
	static ComponentID generateID();
public:
	ComponentID id() const { return m_id; }
	ComponentState state() const { return m_state; }
private:
	ComponentState m_state;
	ComponentID m_id;
	static uint32_t s_globalComponentID;
};

template<typename T>
inline ComponentID Component::generateID()
{
	// Removing bitmask usage of componentID, could let us increase this limit.
	AKA_ASSERT(s_globalComponentID < 63, "Too many components registered.");
	// This need to be called by each components in the same thread to be safe. 
	// Could add a mutex to be safe ?
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	static ComponentID s_localComponentID = static_cast<ComponentID>(s_globalComponentID++);
	return s_localComponentID;
}

};