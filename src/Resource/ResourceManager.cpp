#include <Aka/Resource/ResourceManager.h>

#include <nlohmann/json.hpp>

namespace aka {

ResourceAllocator<Texture> ResourceManager::textures;
ResourceAllocator<Mesh> ResourceManager::meshes;
ResourceAllocator<AudioStream> ResourceManager::audios;
ResourceAllocator<Font> ResourceManager::fonts;
ResourceAllocator<Buffer> ResourceManager::buffers;

void ResourceManager::parse(const Path& path)
{
	std::string content = File::readString(path);
	nlohmann::json json = nlohmann::json::parse(content);
	for (auto& textureKV : json["images"].items())
	{
		Resource<Texture> tex;
		nlohmann::json content = textureKV.value();
		TextureType type = (TextureType)content["type"].get<uint32_t>();
		switch (type)
		{
		case TextureType::Texture2D: {
			uint32_t width = content["width"].get<uint32_t>();
			uint32_t height = content["height"].get<uint32_t>();
			textures.load(textureKV.key(), Path(content["path"].get<std::string>()));
			break;
		}
		case TextureType::TextureCubemap: {
			uint32_t width = content["width"].get<uint32_t>();
			uint32_t height = content["height"].get<uint32_t>();
			Image faces[6];
			for (int i = 0; i < 6; i++)
				faces[i] = Image::load(content["paths"][i]);
			textures.load(
				textureKV.key(),
				Texture::createCubemap(
					width, height,
					TextureFormat::RGBA8,
					TextureFlag::None,
					Sampler{},
					faces[0].bytes.data(),
					faces[1].bytes.data(),
					faces[2].bytes.data(),
					faces[3].bytes.data(),
					faces[4].bytes.data(),
					faces[5].bytes.data()
				)
			);
			break;
		}
		default:
			Logger::error("Texture format not supported");
			break;
		}
	}
}

void ResourceManager::serialize(const Path& path)
{
	nlohmann::json json = nlohmann::json();
	json["images"] = nlohmann::json::object();
	for (auto& texture : textures)
	{
		nlohmann::json node = nlohmann::json::object();
		node["path"] = texture.second.path.cstr();
		json["images"][texture.first.cstr()] = node;
		if (texture.second.loaded != texture.second.updated)
			Resource<Texture>::save(texture.second);
	}
	json["audios"] = nlohmann::json::object();
	for (auto& audio : audios)
	{
		nlohmann::json node = nlohmann::json::object();
		node["path"] = audio.second.path.cstr();
		json["audios"][audio.first.cstr()] = node;
		if (audio.second.loaded != audio.second.updated)
			Resource<AudioStream>::save(audio.second);
	}
	// ...
	std::string str = json.dump(4);
	File::writeString(path.str(), str);
}


};