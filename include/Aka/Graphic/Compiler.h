#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Graphic/Shader.h>

#include <vector>

namespace aka {
namespace gfx {

class Compiler
{
public:
	Compiler();

	// Compile the shader to SPIRV
	bool parse(const Path& path, ShaderType shader, const char** defines = nullptr, size_t defineCount = 0);
	// Export it and generate a shader for given API
	Blob compile(GraphicAPI api);
	// Set the SPIRV code from bytes
	void set(const void* bytes, size_t count);
	// Get shader bindings from compiler
	ShaderBindingState getShaderBindings(uint32_t set);
	// Get vertex bindings from compiler
	VertexBindingState getVertexBindings();
private:
	// Export to HLSL 5.0 for D3D11 backend
	Blob compileHLSL50(const VertexAttribute* attributes, size_t count);
	// Export to GLSL 330 for OpenGL backend
	Blob compileGLSL330();
	// Export to SPIR-V for Vulkan backend
	Blob compileSPV();
private:
	std::vector<uint32_t> m_spirv;
};


};
};
