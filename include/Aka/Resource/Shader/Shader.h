#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Pipeline.h>
#include <Aka/Core/Container/Array.h>

namespace aka {

using ShaderType = gfx::ShaderType;

using ShaderBlob = Blob;

struct ShaderData
{
	String entryPoint;
	Vector<gfx::ShaderBindingState> sets;
	gfx::VertexAttributeState vertices;
};

struct ShaderKey
{
	Path path;
	Vector<String> macros;

	ShaderType type;
	String entryPoint;

	static ShaderKey generate(const Path& path, ShaderType type);
	static ShaderKey fromString(const String& shader, ShaderType type);
};

struct ProgramKey
{
	Vector<ShaderKey> shaders;
};

bool operator<(const ShaderKey& lhs, const ShaderKey& rhs);
bool operator==(const ShaderKey& lhs, const ShaderKey& rhs);

bool operator<(const ProgramKey& lhs, const ProgramKey& rhs);
bool operator==(const ProgramKey& lhs, const ProgramKey& rhs);

};

template <>
struct std::hash<aka::ShaderKey>
{
	size_t operator()(const aka::ShaderKey& key) const
	{
		size_t hash = 0;
		aka::hash(hash, key.entryPoint.cstr(), key.entryPoint.length());
		for (auto& shader : key.macros)
			aka::hash(hash, shader.cstr(), shader.length());
		aka::hash(hash, key.path.cstr(), key.path.length());
		aka::hash(hash, key.type);
		return hash;
	}
};

template <>
struct std::hash<aka::ProgramKey>
{
	size_t operator()(const aka::ProgramKey& key) const
	{
		size_t hash = 0;
		for (auto& shader : key.shaders)
			aka::hashCombine(hash, shader);
		return hash;
	}
};