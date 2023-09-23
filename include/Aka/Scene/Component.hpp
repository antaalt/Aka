#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

class Renderer;

enum class ComponentType : uint32_t
{
	Unknown, 

	StaticMeshComponent,
	CameraComponent,
	//RigidBodyComponent,
	//ColliderComponent,
	SpriteAnimatorComponent,
	AudioComponent,


	First = StaticMeshComponent,
	Last = AudioComponent,
};

enum class ComponentTypeMask
{
	None					= 0,

	StaticMeshComponent		= 1 << EnumToIndex(ComponentType::StaticMeshComponent),
	CameraComponent			= 1 << EnumToIndex(ComponentType::CameraComponent),
	SpriteAnimatorComponent = 1 << EnumToIndex(ComponentType::SpriteAnimatorComponent),
	AudioComponent			= 1 << EnumToIndex(ComponentType::AudioComponent),

	Last = AudioComponent,
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ComponentTypeMask);

template <typename T>
struct ComponentTrait { static const ComponentType type = ComponentType::Unknown; };

class Component
{
public:
	Component(ComponentType type);
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

public:
	ComponentType type() const { return m_type; }
private:
	ComponentType m_type;
};

};