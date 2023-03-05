#pragma once

#include <stdint.h>

#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

enum class ShaderType : uint8_t
{
	Vertex,
	Fragment,
	Compute,
	Geometry,
	TessControl,
	TessEvaluation,

	Count,

	Undefined = 0,
};

enum class ShaderMask : uint8_t
{
	None            = 0,
	Vertex          = (1 << EnumToIntegral(ShaderType::Vertex)),
	Fragment        = (1 << EnumToIntegral(ShaderType::Fragment)),
	Compute         = (1 << EnumToIntegral(ShaderType::Compute)),
	Geometry        = (1 << EnumToIntegral(ShaderType::Geometry)),
	TessControl     = (1 << EnumToIntegral(ShaderType::TessControl)),
	TessEvaluation  = (1 << EnumToIntegral(ShaderType::TessEvaluation)),
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