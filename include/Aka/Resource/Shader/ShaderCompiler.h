#pragma once

#include <Aka/OS/Path.h>
#include <Aka/Resource/Shader/Shader.h>

namespace aka {

class ShaderCompiler
{
public:
	ShaderCompiler();
	~ShaderCompiler();

	// Generate blob from a variant
	ShaderCompilationResult compile(const ShaderKey& key);
	// Get shader data for a specific entry point
	ShaderReflectionData reflect(const ShaderCompilationResult& blob, const char* entryPoint);
private:

};

};