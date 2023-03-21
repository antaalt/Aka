#pragma once

#include <stdint.h>

#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

enum class ShaderType : uint8_t
{
	Unknown,

	Vertex,
	Fragment,
	Compute,
	Geometry,
	TessControl,
	TessEvaluation,

	First = Vertex,
	Last = TessEvaluation,
};

enum class ShaderMask : uint8_t
{
	None            = 0,
	Vertex          = 1 << EnumToIndex(ShaderType::Vertex),
	Fragment        = 1 << EnumToIndex(ShaderType::Fragment),
	Compute         = 1 << EnumToIndex(ShaderType::Compute),
	Geometry        = 1 << EnumToIndex(ShaderType::Geometry),
	TessControl     = 1 << EnumToIndex(ShaderType::TessControl),
	TessEvaluation  = 1 << EnumToIndex(ShaderType::TessEvaluation),
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ShaderMask)

ShaderMask getShaderMask(ShaderType type);

struct Shader;
using ShaderHandle = ResourceHandle<Shader>;

struct Shader : Resource
{
	Shader(const char* name, ShaderType type);

	ShaderType type;

	static ShaderHandle create(ShaderType type, const void* content, size_t size);
	static void destroy(ShaderHandle shader);
};

};
};