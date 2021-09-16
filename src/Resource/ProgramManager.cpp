#include <Aka/Resource/ProgramManager.h>

#include <Aka/Core/Event.h>
#include <Aka/OS/Logger.h>
#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/Resource/ResourceManager.h>

#include <nlohmann/json.hpp>


namespace aka {

std::vector<ProgramManager::ShaderInfo> ProgramManager::m_shaders;
std::vector<ProgramManager::ProgramInfo> ProgramManager::m_programs;

Shader::Ptr ProgramManager::get(const String& name)
{
	for (ProgramInfo& info : m_programs)
		if (info.name == name)
			return info.program;
	return nullptr;
}

ShaderHandle ProgramManager::getShader(const String& name)
{
	for (ShaderInfo& info : m_shaders)
		if (info.name == name)
			return info.shader;
	return ShaderHandle(0);
}

bool ProgramManager::reload(const String& name)
{
	for (ShaderInfo& info : m_shaders)
	{
		if (info.name == name)
		{
			ShaderHandle out = compile(info.path, info.type);
			if (out.value() == 0)
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
	if (!File::read(path, &str))
		return false;
	try
	{
		nlohmann::json json = nlohmann::json::parse(str.cstr());
		if (json["asset"]["version"].get<std::string>() != "0.1")
			return false;
		for (auto& element : json["shaders"].items())
		{
			ShaderInfo info;
			info.loaded = Time::unixtime();
			info.name = element.key();
			info.path = element.value().get<std::string>().c_str();
			String ext = File::extension(info.path);
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
			if (!File::read(path, &str))
				return false;
			info.shader = compile(info.path, info.type);
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
				std::vector<VertexAttribute> attributes;
				for (auto& attribute : element.value()["attributes"])
				{
					VertexAttribute att;
					att.semantic = (VertexSemantic)attribute["semantic"].get<uint32_t>();
					att.format = (VertexFormat)attribute["format"].get<uint32_t>();
					att.type = (VertexType)attribute["type"].get<uint32_t>();
					attributes.push_back(att);
				}
				info.program = Shader::createVertexProgram(getShaderInfo(info.vert).shader, getShaderInfo(info.frag).shader, attributes.data(), attributes.size());
			}
			else if (info.comp.length() > 0)
			{
				info.program = Shader::createComputeProgram(getShaderInfo(info.comp).shader);
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
	// TODO
	return false;
}

void ProgramManager::update()
{
	// TODO execute this only once per second ?
	// TODO use a file watcher
	for (ProgramInfo& programInfo : m_programs)
	{
		if (programInfo.vert.length() > 0 && programInfo.frag.length())
		{
			ShaderInfo& vertInfo = getShaderInfo(programInfo.vert);
			ShaderInfo& fragInfo = getShaderInfo(programInfo.frag);
			bool vertUpdated = File::lastWrite(vertInfo.path) > vertInfo.loaded;
			bool fragUpdated = File::lastWrite(fragInfo.path) > fragInfo.loaded;
			if (vertUpdated || fragUpdated)
			{
				bool compiled = false;
				if (vertUpdated)
				{
					AKA_ASSERT(vertInfo.type == ShaderType::Vertex, "Invalid shader type");
					ShaderHandle shader = compile(vertInfo.path, vertInfo.type);
					if (shader.value() != 0)
					{
						compiled = true;
						vertInfo.shader = shader;
					}
					vertInfo.loaded = Time::unixtime();
				}
				if (fragUpdated)
				{
					AKA_ASSERT(fragInfo.type == ShaderType::Fragment, "Invalid shader type");
					ShaderHandle shader = compile(fragInfo.path, fragInfo.type);
					if (shader.value() != 0)
					{
						compiled = true;
						fragInfo.shader = shader;
					}
					fragInfo.loaded = Time::unixtime();
				}
				if (compiled)
				{
					std::vector<VertexAttribute> att;
					for (uint32_t i = 0; i < programInfo.program->getAttributeCount(); i++)
						att.push_back(programInfo.program->getAttribute(i));
					programInfo.program = Shader::createVertexProgram(vertInfo.shader, fragInfo.shader, att.data(), att.size());
					EventDispatcher<ProgramReloadedEvent>::emit(ProgramReloadedEvent{ programInfo.name, programInfo.program });
				}
			}
		}
		if (programInfo.comp.length() > 0)
		{
			ShaderInfo& compInfo = getShaderInfo(programInfo.comp);
			if (File::lastWrite(compInfo.path) > compInfo.loaded)
			{
				AKA_ASSERT(compInfo.type == ShaderType::Compute, "Invalid shader type");
				ShaderHandle shader = compile(compInfo.path, compInfo.type);
				if (shader.value() != 0)
				{
					compInfo.shader = shader;
					programInfo.program = Shader::createComputeProgram(compInfo.shader);
					EventDispatcher<ProgramReloadedEvent>::emit(ProgramReloadedEvent{ programInfo.name, programInfo.program });
				}
				compInfo.loaded = Time::now();
			}
		}
	}
	EventDispatcher<ProgramReloadedEvent>::dispatch();
}

ShaderHandle ProgramManager::compile(const Path& path, ShaderType type)
{
	static Path glslccPath = ResourceManager::path("shaders/glslcc.exe");
	if (!File::exist(glslccPath))
	{
		Logger::error("glslcc.exe not found.");
		return ShaderHandle(0);
	}
	// TODO Look up in cache before (Do it before calling compile).
	String command = glslccPath.cstr();
	//command.append(" -S"); // silent
	//command.append(" -I path/to/include/dir"); // includes
#if !defined(AKA_DEBUG)
	command.append(" -O"); // optimize
#endif
	String ext;
	switch (type)
	{
	case ShaderType::Vertex:
		ext = "vs";
		command.append(" -v ");
		break;
	case ShaderType::Fragment:
		ext = "fs";
		command.append(" -f ");
		break;
	case ShaderType::Compute:
		ext = "cs";
		command.append(" -c ");
		break;
	default:
		Logger::error("Shader type not supported.");
		return ShaderHandle(0);
	}
	command.append(path.cstr());
	String name = File::basename(path);
	String extension = File::extension(path);
	Path compiledPath;
	switch (GraphicBackend::api())
	{
	case GraphicApi::OpenGL:
		compiledPath = "./library/shaders/GL/" + name + "." + extension + ".glsl";
		command.append(" --output=");
		command.append(compiledPath.cstr());
		command.append(" -l glsl -p 330");
		compiledPath = "./library/shaders/GL/" + name + "_" + ext + "." + extension + ".glsl";
		break;
	case GraphicApi::DirectX11:
		compiledPath = "./library/shaders/D3D/" + name + ".hlsl";
		command.append(" -o ");
		command.append(compiledPath.cstr());
		command.append(" -l hlsl");
		compiledPath = "./library/shaders/D3D/" + name + "_" + ext + ".hlsl";
		break;
	default:
		Logger::error("API not supported.");
		return ShaderHandle(0);
	}
	// Recompile only if cached shader older than shader
	if (!File::exist(compiledPath) || File::lastWrite(compiledPath) < File::lastWrite(path))
	{
		int status = std::system(command.cstr());
		if (status < 0)
		{
			Logger::error("Failed to execute glslcc.");
			return ShaderHandle(0);
		}
		else
		{
			Logger::debug("Shader ", File::name(path), " successfully compiled.");
		}
	}
	else
	{
		Logger::debug("Shader ", File::name(path), " loaded from cache.");
	}
	String str;
	if (!File::read(compiledPath, &str))
	{
		Logger::error("Could not read ", compiledPath);
		return ShaderHandle(0);
	}
	return Shader::compile(str.cstr(), type);
}

};