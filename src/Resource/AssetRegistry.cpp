#include <Aka/Resource/AssetRegistry.h>

#include <Aka/Core/Application.h>
#include <Aka/Resource/Asset.h>
#include <Aka/Core/Config.h>
#include <Aka/OS/OS.h>
#include <Aka/OS/Logger.h>

#include <map>
#include <nlohmann/json.hpp>

namespace aka {

void AssetRegistry::parse(const Path& path)
{
	// TODO parse library.json
	String content;
	if (!OS::File::read(path, &content))
		Logger::error("Failed to load : ", path);
	try
	{
		Asset asset;
		asset.resource = nullptr;
		nlohmann::json json = nlohmann::json::parse(content.cstr());
		{ // Buffers
			asset.type = ResourceType::Buffer;
			for (auto& buffer : json["buffers"].items())
			{
				asset.path = Path(buffer.value().get<std::string>());
				asset.diskSize = OS::File::size(asset.path);
				add(buffer.key().c_str(), asset);
			}
		}
		{ // Meshes
			asset.type = ResourceType::Mesh;
			for (auto& mesh : json["meshes"].items())
			{
				asset.path = Path(mesh.value().get<std::string>());
				asset.diskSize = OS::File::size(asset.path);
				add(mesh.key().c_str(), asset);
			}
		}
		{ // Textures
			asset.type = ResourceType::Texture;
			for (auto& texture : json["images"].items())
			{
				asset.path = Path(texture.value().get<std::string>());
				asset.diskSize = OS::File::size(asset.path);
				add(texture.key().c_str(), asset);
			}
		}
		{ // Audios
			asset.type = ResourceType::Audio;
			for (auto& audio : json["audios"].items())
			{
				asset.path = Path(audio.value().get<std::string>());
				asset.diskSize = OS::File::size(asset.path);
				add(audio.key().c_str(), asset);
			}
		}
		{ // Fonts
			asset.type = ResourceType::Font;
			for (auto& font : json["fonts"].items())
			{
				asset.path = Path(font.value().get<std::string>());
				asset.diskSize = OS::File::size(asset.path);
				add(font.key().c_str(), asset);
			}
		}
	}
	catch (const nlohmann::json::exception& e)
	{
		Logger::error("Failed to parse " + OS::File::name(path) + " : ", e.what());
	}
}
void AssetRegistry::serialize(const Path& path)
{
	static const std::map<ResourceType, const char*> types = {
		{ ResourceType::Texture, "textures"},
		{ ResourceType::Mesh, "meshes"},
		{ ResourceType::Audio, "audios"},
		{ ResourceType::Font, "textfontsures"},
		{ ResourceType::Buffer, "buffers"},
	};
	String str;
	try
	{
		nlohmann::json json = nlohmann::json();
		for (const auto& type : types)
		{
			json[type.second] = nlohmann::json::object();
		}
		for (const auto& pair : m_assets)
		{
			const String& name = pair.first;
			const Asset& asset = pair.second;
			const char* typeString = types.find(asset.type)->second;
			json[typeString][name.cstr()] = asset.path.cstr();
		}
#if defined(AKA_DEBUG)
		str = json.dump(4); // Pretty print
#else
		str = json.dump();
#endif
	}
	catch (const nlohmann::json::exception& e)
	{
		Logger::error("Failed to serialize " + OS::File::name(path) + " : ", e.what());
	}
	OS::File::write(path, str);
}
Asset AssetRegistry::find(const char* name)
{
	auto it = m_assets.find(name);
	if (it == m_assets.end())
		return Asset{}; // Null asset
	return it->second;
}
void AssetRegistry::add(const char* name, Asset asset)
{
	asset.name = name; // TODO not the best but asset need a name that is fitting the key.
	auto it = m_assets.insert(std::make_pair(name, asset));
	if (it.second)
		return; // Failed to add
}
void AssetRegistry::remove(const char* name)
{
	auto itFind = m_assets.find(name);
	if (itFind == m_assets.end())
		return; // Does not exist
	auto it = m_assets.erase(itFind);
}

void AssetRegistry::clear()
{
	gfx::GraphicDevice* device = Application::app()->graphic();
	for (auto& asset : m_assets)
	{
		asset.second.unload(device);
	}
	m_assets.clear();
}

};