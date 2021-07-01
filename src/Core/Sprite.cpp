#include <Aka/Core/Sprite.h>

#include <Aka/Core/Aseprite.h>
#include <Aka/OS/Image.h>
#include <Aka/OS/Stream/FileStream.h>

#include <cstring>

namespace aka {

Sprite::Frame convertFrame(const Aseprite& ase, const Aseprite::Frame& aseFrame)
{
	std::vector<Aseprite::Color32> aseImage = aseFrame.image(ase);
	Image image(ase.width, ase.height, 4);
	memcpy(image.bytes.data(), aseImage[0].data, image.bytes.size());
#if defined(ORIGIN_BOTTOM_LEFT)
	image.flip();
#endif
	// Set frame
	Sprite::Frame frame;
	frame.duration = Time::Unit::milliseconds(aseFrame.duration);
	frame.width = ase.width;
	frame.height = ase.height;
	// Generate texture
	frame.texture = Texture::create2D(
		ase.width, ase.height,
		TextureFormat::UnsignedByte,
		TextureComponent::RGBA,
		TextureFlag::None,
		Sampler::nearest(),
		image.bytes.data()
	);
	return frame;
}

Sprite Sprite::parse(Stream& stream)
{
	Aseprite ase = Aseprite::parse(stream);
	Sprite sprite;
	if (ase.tags.size() == 0)
	{
		// Default animation
		Sprite::Animation animation;
		animation.name = "Idle";
		for (Aseprite::Frame& aseFrame : ase.frames)
			animation.frames.push_back(convertFrame(ase, aseFrame));
		sprite.animations.push_back(animation);
	}
	else
	{
		for (Aseprite::Tag& aseTag : ase.tags)
		{
			Sprite::Animation animation;
			animation.name = aseTag.name;
			for (Aseprite::Word iFrame = aseTag.from; iFrame <= aseTag.to; iFrame++)
				animation.frames.push_back(convertFrame(ase, ase.frames[iFrame]));
			sprite.animations.push_back(animation);
		}
	}
	return sprite;
}

};