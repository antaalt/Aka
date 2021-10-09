#include <Aka/Resource/ResourceManager.h>

#include <Aka/OS/OS.h>

#include <nlohmann/json.hpp>

namespace aka {

Path ResourceManager::assetPath = OS::cwd() + Path("asset/");

void ResourceManager::parse(const Path& path)
{
	String content;
	if (!OS::File::read(path, &content))
		Logger::error("Failed to load : ", path);
	try
	{
		nlohmann::json json = nlohmann::json::parse(content.cstr());
		for (auto& buffer : json["buffers"].items())
		{
			buffers.load(String(buffer.key()), Path(buffer.value().get<std::string>()));
		}
		for (auto& mesh : json["meshes"].items())
		{
			meshes.load(String(mesh.key()), Path(mesh.value().get<std::string>()));
		}
		for (auto& texture : json["images"].items())
		{
			textures.load(String(texture.key()), Path(texture.value().get<std::string>()));
		}
		for (auto& audio : json["audios"].items())
		{
			audios.load(String(audio.key()), Path(audio.value().get<std::string>()));
		}
		for (auto& font : json["fonts"].items())
		{
			fonts.load(String(font.key()), Path(font.value().get<std::string>()));
		}
	}
	catch (const nlohmann::json::exception& e)
	{
		Logger::error("Failed to parse " + OS::File::name(path) + " : ", e.what());
	}
}

void ResourceManager::serialize(const Path& path)
{
	String str;
	try
	{
		nlohmann::json json = nlohmann::json();
		json["images"] = nlohmann::json::object();
		for (auto& texture : textures)
		{
			json["images"][texture.first.cstr()] = texture.second.path.cstr();
		}
		json["meshes"] = nlohmann::json::object();
		for (auto& mesh : meshes)
		{
			json["meshes"][mesh.first.cstr()] = mesh.second.path.cstr();
		}
		json["audios"] = nlohmann::json::object();
		for (auto& audio : audios)
		{
			json["audios"][audio.first.cstr()] = audio.second.path.cstr();
		}
		json["fonts"] = nlohmann::json::object();
		for (auto& font : fonts)
		{
			json["fonts"][font.first.cstr()] = font.second.path.cstr();
		}
		json["buffers"] = nlohmann::json::object();
		for (auto& buffer : buffers)
		{
			json["buffers"][buffer.first.cstr()] = buffer.second.path.cstr();
		}
		str = json.dump(4);
	}
	catch (const nlohmann::json::exception& e)
	{
		Logger::error("Failed to serialize " + OS::File::name(path) + " : ", e.what());
	}
	OS::File::write(path, str);
}


Path ResourceManager::path(const Path& path)
{
	return OS::normalize(assetPath + path);
}


};