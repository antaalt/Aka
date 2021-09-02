#include <Aka/Scene/ResourceAllocator.h>

namespace aka {

Resource<Texture> Resource<Texture>::load(const Path& path)
{
	Resource<Texture> res;
	res.path = path;
	res.loaded = Time::now();
	res.updated = Time::now();
	Image img = Image::load(path);
	if (img.bytes.size() == 0)
		return Resource<Texture>{};
	res.size = img.bytes.size();
	res.resource = Texture::create2D(img.width, img.height, TextureFormat::RGBA8, TextureFlag::None, Sampler{}, img.bytes.data());
	return res;
}
void Resource<Texture>::save(const Path& path, const Resource<Texture>& resource)
{
	Image img;
	img.width = resource.resource->width();
	img.height = resource.resource->height();
	img.components = 4;
	img.bytes.resize(4 * img.width * img.height);
	resource.resource->download(img.bytes.data());
	img.save(path);
}

Resource<AudioStream> Resource<AudioStream>::load(const Path& path)
{
	Resource<AudioStream> res;
	res.path = path;
	res.loaded = Time::now();
	res.updated = Time::now();
	res.resource = AudioStream::openStream(path);
	res.size = 0;
	if (res.resource == nullptr)
		return Resource<AudioStream>{};
	return res;
}
void Resource<AudioStream>::save(const Path& path, const Resource<AudioStream>& resource)
{
	// TODO
}

Resource<Mesh> Resource<Mesh>::load(const Path& path)
{
	Resource<Mesh> res;
	res.path = path;
	res.loaded = Time::now();
	res.updated = Time::now();
	res.resource = Mesh::create(); // TODO load mesh info
	res.size = 0;
	if (res.resource == nullptr)
		return Resource<Mesh>{};
	return res;
}
void Resource<Mesh>::save(const Path& path, const Resource<Mesh>& resource)
{
	// TODO
}

template struct Resource<Texture>;
template struct Resource<AudioStream>;
template struct Resource<Mesh>;

};