#if defined(AKA_USE_OPENGL)
#pragma once

#include "GLContext.h"

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Shader.h>

namespace aka {

class GLShader : public Shader
{
	friend class GLDevice;
	friend class GLProgram;
public:
	GLShader(GLuint shaderID);
	~GLShader();

	static Shader::Ptr compileGL(const char* content, ShaderType type);
private:
	GLuint m_shaderID;
};

class GLProgram : public Program
{
	friend class GLDevice;
	friend class GLMaterial;
public:
	GLProgram(Shader::Ptr vertex, Shader::Ptr fragment, Shader::Ptr geometry, Shader::Ptr compute, const VertexAttribute* attributes, size_t count);
	~GLProgram();
private:
	GLuint m_programID;
};


};

#endif