#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

class Renderer;

enum class ComponentID : uint32_t { Invalid = (uint32_t)-1 };

class Component
{
public:
	Component(ComponentID id);
	virtual ~Component() {}

protected:
	friend class Node;
	virtual void onAttach() {}
	virtual void onDetach() {}
	virtual void onCreate(Renderer* _device) {}
	virtual void onDestroy(Renderer* _device) {}
	virtual void onUpdate(Time deltaTime) {}
	virtual void onFixedUpdate(Time deltaTime) {}
	virtual void onRender(Renderer* _device, gfx::Frame* _frame) {}
protected:
	template <typename T>
	static ComponentID generateID();
public:
	ComponentID id() const { return m_id; }
private:
	ComponentID m_id;
	static uint32_t s_globalComponentID;
};

template<typename T>
inline ComponentID Component::generateID()
{
	// This need to be called by each components in the same thread to be safe. 
	// Could add a mutex to be safe ?
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	static ComponentID s_localComponentID = static_cast<ComponentID>(s_globalComponentID++);
	return s_localComponentID;
}

};