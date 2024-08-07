#pragma once

#include <Aka/Core/Hash.hpp>
#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Pipeline.h>
#include <Aka/Core/Container/Array.h>
#include <Aka/Resource/AssetPath.hpp>

namespace aka {

using ShaderType = gfx::ShaderType;

using ShaderBlob = Blob;

struct ShaderCompilationResult
{
	ShaderBlob blob;
	Vector<AssetPath> dependencies;
};

struct ShaderReflectionData
{
	String entryPoint;
	Vector<gfx::ShaderBindingState> sets;
	gfx::VertexState vertices;
	Vector<gfx::ShaderConstant> constants;
	Vector<AssetPath> dependencies;
};

struct Macro
{
	String key;
	String value;
};

struct ShaderKey
{
	AssetPath path;
	Vector<Macro> macros;

	ShaderType type;
	String entryPoint = "main";

	ShaderKey& setPath(const AssetPath& _path) { path = _path; return *this; }
	ShaderKey& setType(ShaderType _type) { type = _type; return *this; }
	ShaderKey& setEntryPoint(const String& _entryPoint) { entryPoint = _entryPoint; return *this; }
	ShaderKey& addMacro(const String& _macro) { macros.append(Macro{ _macro, "" }); return *this; }
	ShaderKey& addMacro(const String& _macro, const String& _value) { macros.append(Macro{ _macro, _value }); return *this; }

	static ShaderKey generate(const AssetPath& path, ShaderType type);
};

struct ProgramKey
{
	Vector<ShaderKey> shaders;

	ProgramKey& add(const ShaderKey& key) { shaders.append(key); return *this; }
};

bool operator==(const Macro& lhs, const Macro& rhs);
bool operator!=(const Macro& lhs, const Macro& rhs);

bool operator<(const ShaderKey& lhs, const ShaderKey& rhs);
bool operator==(const ShaderKey& lhs, const ShaderKey& rhs);
bool operator!=(const ShaderKey& lhs, const ShaderKey& rhs);

bool operator<(const ProgramKey& lhs, const ProgramKey& rhs);
bool operator==(const ProgramKey& lhs, const ProgramKey& rhs);


std::ostream& operator<<(std::ostream& os, const ShaderKey& key);

};

template <>
struct std::hash<aka::ShaderKey>
{
	size_t operator()(const aka::ShaderKey& key) const
	{
		size_t hash = 0;
		aka::hash::fnv(hash, key.entryPoint.cstr(), key.entryPoint.length());
		for (auto& shader : key.macros)
		{
			aka::hash::fnv(hash, shader.key.cstr(), shader.key.length());
			aka::hash::fnv(hash, shader.value.cstr(), shader.value.length());
		}
		aka::hash::fnv(hash, key.path.cstr(), key.path.size());
		aka::hash::fnv(hash, &key.type, sizeof(aka::gfx::ShaderType));
		return hash;
	}
};

template <>
struct std::hash<aka::ProgramKey>
{
	size_t operator()(const aka::ProgramKey& key) const
	{
		size_t hash = 0;
		std::hash<aka::ShaderKey> hasher;
		for (auto& shader : key.shaders)
			aka::hash::combine(hash, hasher(shader));
		return hash;
	}
};