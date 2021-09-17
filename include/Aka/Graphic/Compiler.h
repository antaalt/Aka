#pragma once

#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/Graphic/Shader.h>

#include <vector>

namespace aka {

class Compiler
{
public:
	Compiler();

	// Compile the shader to SPIRV
	bool parse(const Path& path, ShaderType shader, const char** defines = nullptr, size_t defineCount = 0);
	// Export it and generate a shader for given API
	String compile(GraphicApi api, const VertexAttribute* attributes, size_t count);
private:
	// Export to HLSL 5.0 for D3D11 backend
	String compileHLSL50(const VertexAttribute* attributes, size_t count);
	// Export to GLSL 330 for OpenGL backend
	String compileGLSL330();
private:
	std::vector<uint32_t> m_spirv;
};


};