#include <Aka/Core/Sprite.h>

#include <Aka/Core/Aseprite.h>

namespace aka {

Sprite Sprite::parse(const Path& path)
{
	std::vector<uint8_t> bytes = BinaryFile::load(path);
	Aseprite ase = Aseprite::parse(bytes);
	Sprite sprite;
	if (ase.tags.size() == 0)
	{
		// Default animation
		Sprite::Animation animation;
		animation.name = "Idle";
		for (Aseprite::Frame& aseFrame : ase.frames)
		{
			std::vector<Aseprite::Color32> image = aseFrame.image(ase);
			// Set frame
			Sprite::Frame frame;
			frame.duration = Time::Unit::milliseconds(aseFrame.duration);
			frame.width = ase.width;
			frame.height = ase.height;
			// Generate texture
			Sampler sampler{};
			sampler.filterMag = aka::Sampler::Filter::Nearest;
			sampler.filterMin = aka::Sampler::Filter::Nearest;
			sampler.wrapS = aka::Sampler::Wrap::Clamp;
			sampler.wrapT = aka::Sampler::Wrap::Clamp;
			frame.texture = Texture::create(ase.width, ase.height, Texture::Format::Rgba, image[0].data, sampler);
			animation.frames.push_back(frame);
		}
		sprite.animations.push_back(animation);
	}
	else
	{
		for (Aseprite::Tag& aseTag : ase.tags)
		{
			Sprite::Animation animation;
			animation.name = aseTag.name;
			for (Aseprite::Word iFrame = aseTag.from; iFrame <= aseTag.to; iFrame++)
			{
				Aseprite::Frame& aseFrame = ase.frames[iFrame];
				std::vector<Aseprite::Color32> image = aseFrame.image(ase);
				// Set frame
				Sprite::Frame frame;
				frame.duration = Time::Unit::milliseconds(aseFrame.duration);
				frame.width = ase.width;
				frame.height = ase.height;
				// Generate texture
				Sampler sampler{};
				sampler.filterMag = aka::Sampler::Filter::Nearest;
				sampler.filterMin = aka::Sampler::Filter::Nearest;
				sampler.wrapS = aka::Sampler::Wrap::Clamp;
				sampler.wrapT = aka::Sampler::Wrap::Clamp;
				frame.texture = Texture::create(ase.width, ase.height, Texture::Format::Rgba, image[0].data, sampler);
				animation.frames.push_back(frame);
			}
			sprite.animations.push_back(animation);
		}
	}
	return sprite;
}

};