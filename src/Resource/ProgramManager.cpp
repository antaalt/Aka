#include <Aka/Resource/ProgramManager.h>

#include <Aka/Core/Event.h>
#include <Aka/Core/Application.h>
#include <Aka/OS/OS.h>
#include <Aka/OS/Logger.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Graphic/Compiler.h>
#include <Aka/Resource/ResourceManager.h>

#include <nlohmann/json.hpp>


namespace aka {

Program* ProgramManager::get(const String& name)
{
	for (ProgramInfo& info : m_programs)
		if (info.name == name)
			return info.program;
	return nullptr;
}

Shader* ProgramManager::getShader(const String& name)
{
	for (ShaderInfo& info : m_shaders)
		if (info.name == name)
			return info.shader;
	return nullptr;
}

bool ProgramManager::reload(const String& name)
{
	for (ShaderInfo& info : m_shaders)
	{
		if (info.name == name)
		{
			Shader* out = compile(info.path, info.type, &info.bindings, &info.vertices);
			if (out == nullptr)
				return false;
			info.shader = out;
			return true;
		}
	}
	return false;
}

bool ProgramManager::parse(const Path& path)
{
	// Get all shaders path & name, as well as program name and shaders. Load them, build them...
	String str;
	if (!OS::File::read(path, &str))
		return false;
	try
	{
		nlohmann::json json = nlohmann::json::parse(str.cstr());
		if (json["asset"]["version"].get<std::string>() != "0.1")
			return false;
		for (auto& element : json["shaders"].items())
		{
			ShaderInfo info;
			info.loaded = Timestamp::now();
			info.name = element.key();
			info.path = element.value()["path"].get<std::string>().c_str();
			String ext = OS::File::extension(info.path);
			// TODO add type in shader.json instead of checking extension.
			if (ext == "vert")
				info.type = ShaderType::Vertex;
			else if (ext == "frag")
				info.type = ShaderType::Fragment;
			else if (ext == "comp")
				info.type = ShaderType::Compute;
			else
			{
				Logger::warn("Invalid shader type. Ignoring.");
				continue;
			}
			/*if (element.value().find("attributes") != element.value().end())
			{
				for (auto& attribute : element.value()["attributes"])
				{
					VertexAttribute att;
					att.semantic = (VertexSemantic)attribute["semantic"].get<uint32_t>();
					att.format = (VertexFormat)attribute["format"].get<uint32_t>();
					att.type = (VertexType)attribute["type"].get<uint32_t>();
					info.attributes.push_back(att);
				}
			}*/
			if (!OS::File::read(path, &str))
				return false;
			info.shader = compile(info.path, info.type, &info.bindings, &info.vertices);
			m_shaders.push_back(info);
		}
		for (auto& element : json["programs"].items())
		{
			ProgramInfo info;
			info.name = element.key();
			if (!element.value()["vertex"].is_null())
				info.vert = element.value()["vertex"].get<std::string>();
			if (!element.value()["fragment"].is_null())
				info.frag = element.value()["fragment"].get<std::string>();
			if (!element.value()["compute"].is_null())
				info.comp = element.value()["compute"].get<std::string>();
			if (info.vert.length() > 0 && info.frag.length() > 0)
			{
				ShaderBindingState vertBindings = getShaderInfo(info.vert).bindings;
				ShaderBindingState fragBindings = getShaderInfo(info.frag).bindings;
				ShaderBindingState bindings = ShaderBindingState::merge(vertBindings, fragBindings);
				info.program = Program::createVertex(
					getShaderInfo(info.vert).shader,
					getShaderInfo(info.frag).shader,
					bindings
				);
			}
			else if (info.comp.length() > 0)
			{
				info.program = Program::createCompute(
					getShaderInfo(info.comp).shader
				);
			}
			m_programs.push_back(info);
		}
		return true;
	}
	catch (const nlohmann::json::exception& e)
	{
		Logger::error(e.what());
		return false;
	}
	
}

bool ProgramManager::serialize(const Path& path)
{
	try
	{
		nlohmann::json json = nlohmann::json::object();
		json["asset"]["version"] = "0.1";
		json["shaders"] = nlohmann::json::object();
		nlohmann::json& shaderJSON = json["shaders"];
		for (ShaderInfo& info : m_shaders)
		{
			shaderJSON[info.name] = nlohmann::json::object();
			shaderJSON[info.name]["path"] = info.path.cstr();
			if (info.bindings.count > 0)
			{
				shaderJSON[info.name]["bindings"] = nlohmann::json::array();
				for (size_t i = 0; i < info.bindings.count; i++)
				{
					ShaderBindingLayout& binding = info.bindings.bindings[i];
					nlohmann::json att = nlohmann::json::object();
					att["shaderType"] = (uint32_t)binding.shaderType;
					att["count"] = (uint32_t)binding.count;
					att["type"] = (uint32_t)binding.type;
					shaderJSON[info.name]["bindings"].push_back(att);
				}
			}
			if (info.vertices.count > 0)
			{
				shaderJSON[info.name]["inputs"] = nlohmann::json::array();
				info.vertices.offsets; // TODO move out
				for (size_t i = 0; i < info.vertices.count; i++)
				{
					VertexAttribute& attributes = info.vertices.attributes[i];
					nlohmann::json att = nlohmann::json::object();
					att["semantic"] = (uint32_t)attributes.semantic;
					att["format"] = (uint32_t)attributes.format;
					att["type"] = (uint32_t)attributes.type;
					shaderJSON[info.name]["inputs"].push_back(att);
				}
			}
		}
		json["programs"] = nlohmann::json::object();
		nlohmann::json& programJSON = json["programs"];
		for (ProgramInfo& info : m_programs)
		{
			if (info.vert.length() > 0)
				programJSON[info.name]["vertex"] = info.vert;
			if (info.frag.length() > 0)
				programJSON[info.name]["fragment"] = info.frag;
			if (info.comp.length() > 0)
				programJSON[info.name]["compute"] = info.comp;
		}
		std::string str = json.dump(4);
		Path dir = path.up();
		if (!OS::Directory::exist(dir))
			OS::Directory::create(dir);
		if (!OS::File::write(path, str.c_str()))
			return false;
		return true;
	}
	catch (const nlohmann::json::exception& e)
	{
		Logger::error(e.what());
		return false;
	}
}

void ProgramManager::onReceive(const AppUpdateEvent& event)
{
	// TODO execute this only once per second ?
	// TODO use a file watcher
	for (ProgramInfo& programInfo : m_programs)
	{
		if (programInfo.vert.length() > 0 && programInfo.frag.length())
		{
			ShaderInfo& vertInfo = getShaderInfo(programInfo.vert);
			ShaderInfo& fragInfo = getShaderInfo(programInfo.frag);
			bool vertUpdated = OS::File::lastWrite(vertInfo.path) > vertInfo.loaded;
			bool fragUpdated = OS::File::lastWrite(fragInfo.path) > fragInfo.loaded;
			if (vertUpdated || fragUpdated)
			{
				bool compiled = false;
				if (vertUpdated)
				{
					AKA_ASSERT(vertInfo.type == ShaderType::Vertex, "Invalid shader type");
					Shader* shader = compile(vertInfo.path, vertInfo.type, &vertInfo.bindings, &vertInfo.vertices);
					if (shader != nullptr)
					{
						compiled = true;
						vertInfo.shader = shader;
					}
					vertInfo.loaded = Timestamp::now();
				}
				if (fragUpdated)
				{
					AKA_ASSERT(fragInfo.type == ShaderType::Fragment, "Invalid shader type");
					Shader* shader = compile(fragInfo.path, fragInfo.type, &fragInfo.bindings, &fragInfo.vertices);
					if (shader != nullptr)
					{
						compiled = true;
						fragInfo.shader = shader;
					}
					fragInfo.loaded = Timestamp::now();
				}
				if (compiled)
				{
					ShaderBindingState bindings = ShaderBindingState::merge(vertInfo.bindings, fragInfo.bindings);
					programInfo.program = Program::createVertex(vertInfo.shader, fragInfo.shader, bindings);
					EventDispatcher<ProgramReloadedEvent>::emit(ProgramReloadedEvent{ programInfo.name, programInfo.program });
				}
			}
		}
		if (programInfo.comp.length() > 0)
		{
			ShaderInfo& compInfo = getShaderInfo(programInfo.comp);
			if (OS::File::lastWrite(compInfo.path) > compInfo.loaded)
			{
				AKA_ASSERT(compInfo.type == ShaderType::Compute, "Invalid shader type");
				ShaderBindingState bindings{};
				Shader* shader = compile(compInfo.path, compInfo.type, &bindings, nullptr);
				if (shader != nullptr)
				{
					compInfo.shader = shader;
					programInfo.program = Program::createCompute(compInfo.shader);
					EventDispatcher<ProgramReloadedEvent>::emit(ProgramReloadedEvent{ programInfo.name, programInfo.program });
				}
				compInfo.loaded = Timestamp::now();
			}
		}
	}
	EventDispatcher<ProgramReloadedEvent>::dispatch();
}

Shader* ProgramManager::compile(const Path& path, ShaderType type, ShaderBindingState* bindings, VertexBindingState* vertices)
{
	Application* app = Application::app();
	GraphicDevice* device = app->graphic();
	String compiledPath;
	String fileName;
	switch (device->api())
	{
	case GraphicAPI::OpenGL3:
		compiledPath = "./library/shaders/GL/";
		fileName = OS::File::name(path) + ".glsl";
		break;
	case GraphicAPI::DirectX11:
		compiledPath = "./library/shaders/D3D/";
		fileName = OS::File::name(path) + ".hlsl";
		break;
	case GraphicAPI::Vulkan:
		compiledPath = "./library/shaders/SPV/";
		fileName = OS::File::name(path) + ".spv";
		break;
	default:
		return nullptr;
	}
	String finalPath = compiledPath + fileName;
	Blob shader;
	//if (!OS::File::exist(finalPath) || OS::File::lastWrite(finalPath) < OS::File::lastWrite(path))
	{
		Compiler compiler;
		if (!compiler.parse(path, type))
		{
			Logger::error("Shader ", OS::File::name(path), " failed to compiled.");
			return nullptr;
		}
		if (vertices && type == ShaderType::Vertex)
			*vertices = compiler.getVertexBindings();
		if (bindings)
			*bindings = compiler.getShaderBindings();
		shader = compiler.compile(device->api());
		if (!OS::Directory::exist(compiledPath))
			OS::Directory::create(compiledPath);
		if (!OS::File::write(finalPath, shader))
			Logger::warn("Failed to cache shader");
		Logger::debug("Shader ", OS::File::name(path), " successfully compiled.");
	}
	/*else
	{
		if (!OS::File::read(finalPath, &shader))
		{
			Logger::error("Could not read ", finalPath);
			return nullptr;
		}
		*vertices = {};
		*bindings = {};
		Logger::debug("Shader ", OS::File::name(path), " loaded from cache.");
	}*/
	return Shader::compile(type, shader.data(), shader.size());

}

};