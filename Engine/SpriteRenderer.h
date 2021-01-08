#pragma once

#include "Texture.h"
#include "Geometry.h"
#include "Time.h"
#include "Platform.h"

#include <vector>

namespace app {

struct Sprite {
	Texture* texture;
	vec2f position;
	vec2f size;
	radianf rotation;
};

struct AnimatedSprite
{
	AnimatedSprite() : totalDuration(0) {}

	std::vector<Texture*> textures;
	std::vector<Time::unit> durations;
	Time::unit totalDuration;

	vec2f position;
	vec2f size;
	radianf rotation;

	void addSprite(Texture* texture, Time::unit duration) {
		textures.push_back(texture);
		durations.push_back(duration);
		totalDuration += duration;
	}

	Sprite getSprite(Time::unit time) const {
		ASSERT(textures.size() == durations.size(), "");
		time = time % totalDuration;
		Time::unit duration = 0;
		for (size_t iSprite = 0; iSprite < textures.size(); iSprite++)
		{
			if ((duration += durations[iSprite]) >= time)
			{
				Sprite sprite;
				sprite.texture = textures[iSprite];
				sprite.position = position;
				sprite.size = size;
				sprite.rotation = rotation;
				return sprite;
			}
		}
		// Should not be reached
		Sprite sprite;
		sprite.texture = textures[0];
		sprite.position = position;
		sprite.size = size;
		sprite.rotation = rotation;
		return sprite;
	}
};

class SpriteRenderer
{
public:
	virtual void create() = 0;
	virtual void destroy() = 0;

	void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);

	virtual void render(const Sprite &sprite) = 0;
protected:
	int32_t m_x, m_y;
	uint32_t m_width, m_height;
};

};
