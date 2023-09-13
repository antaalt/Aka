#pragma once

#include <Aka/Scene/World.h>

namespace aka {

struct Serializer
{
	static String serialize(const World& world);

private:
	// Serialize an entity
	String serialize(Serializer& s, Entity entity);
	// Serialize an entity component
	template <typename T>
	String serialize(Serializer& s, T& component);
};


};