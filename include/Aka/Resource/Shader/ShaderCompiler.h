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
	ShaderBlob compile(const ShaderKey& key);
	// Get shader data for a specific entry point
	ShaderData reflect(const ShaderBlob& blob, const char* entryPoint);
private:

};

};