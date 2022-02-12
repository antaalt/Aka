#pragma once

#include <stdint.h>

namespace aka {

enum class ShaderType : uint8_t
{
	None = 0,
	Vertex = (1 << 0),
	Fragment = (1 << 1),
	Compute = (1 << 2),
	Geometry = (1 << 3),
	//TessControl     = (1 << 4),
	//TessEvaluation  = (1 << 5),
};

bool has(ShaderType flags, ShaderType flag);
ShaderType operator&(ShaderType lhs, ShaderType rhs);
ShaderType operator|(ShaderType lhs, ShaderType rhs);

struct Shader
{
	void* native;

	ShaderType type;

	static Shader* compile(ShaderType type, const uint8_t* content, size_t size);
	static void destroy(Shader* shader);
};

};