#pragma once

#include <vector>
#include <memory>

namespace aka {

enum class ShaderType
{
	Vertex = (1 << 0),
	Fragment = (1 << 1),
	Compute = (1 << 2),
	Geometry = (1 << 3),
	//TessControl     = (1 << 4),
	//TessEvaluation  = (1 << 5),
};

class Shader
{
public:
	using Ptr = std::shared_ptr<Shader>;
protected:
	Shader();
	Shader(const Shader&) = delete;
	const Shader& operator=(const Shader&) = delete;
	virtual ~Shader();
public:
	static Shader::Ptr compile(const char* content, ShaderType type);
};

};