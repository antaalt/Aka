#include <Aka/Scene/Serializer.h>

#include <Aka/Scene/Component.h>
#include <Aka/OS/Logger.h>

namespace aka {

struct Transform2D
{
	vec2f pos;
	vec2f size;
};


std::string Serializer::serialize(const World& world)
{
	Serializer serializer;
	world.each([&](Entity entity) {
		serializer.serialize(serializer, entity);
	});
	return std::string();
}

std::string Serializer::serialize(Serializer& s, Entity entity)
{
	/*if (entity.has<Transform2D>()) s.serialize<Transform2D>(s, entity.get<Transform2D>());
	if (entity.has<Transform2D>()) s.serialize<Transform2D>(s, entity.get<Transform2D>());
	if (entity.has<Transform2D>()) s.serialize<Transform2D>(s, entity.get<Transform2D>());
	if (entity.has<Transform2D>()) s.serialize<Transform2D>(s, entity.get<Transform2D>());
	if (entity.has<Transform2D>()) s.serialize<Transform2D>(s, entity.get<Transform2D>());
	if (entity.has<Transform2D>()) s.serialize<Transform2D>(s, entity.get<Transform2D>());
	if (entity.has<Transform2D>()) s.serialize<Transform2D>(s, entity.get<Transform2D>());
	if (entity.has<Transform2D>()) s.serialize<Transform2D>(s, entity.get<Transform2D>());*/
	return std::string();
}

template <typename T>
std::string Serializer::serialize(Serializer& s, T& component)
{
	Logger::warn("Component ", ComponentHandle<T>::name, " not serializable.");
	return std::string();
}

template <>
std::string Serializer::serialize<Transform2D>(Serializer& s, Transform2D& component)
{
	//serializer.write<vec2f>(component.pos);
	return std::string();
}

};