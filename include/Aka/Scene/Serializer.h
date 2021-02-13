#pragma once

#include <Aka/Scene/World.h>

namespace aka {

struct Serializer
{
	static std::string serialize(const World& world);

private:
	// Serialize an entity
	std::string serialize(Serializer& s, Entity entity);
	// Serialize an entity component
	template <typename T>
	std::string serialize(Serializer& s, T& component);
};


};