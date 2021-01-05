#include "GLShader.h"

#include <stdexcept>
#include <vector>
#include <iostream>

namespace app {
namespace gl {

Shader::Shader() :
	id(0)
{

}
Shader::~Shader()
{
	if (id != 0)
		destroy();
}

void Shader::create(const ShaderInfo& info)
{
	this->id = glCreateProgram();
	// Attach shaders
	if (info.vertex != 0 && glIsShader(info.vertex) == GL_TRUE)
		glAttachShader(this->id, info.vertex);
	if (info.frag != 0 && glIsShader(info.frag) == GL_TRUE)
		glAttachShader(this->id, info.frag);
	if (info.compute != 0 && glIsShader(info.compute) == GL_TRUE)
		glAttachShader(this->id, info.compute);

	// link program
	glLinkProgram(this->id);
	GLint linked;
	glGetProgramiv(this->id, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(this->id, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(this->id, maxLength, &maxLength, &errorLog[0]);
		std::string str(errorLog.begin(), errorLog.end());
		// Exit with failure.
		glDeleteProgram(this->id); // Don't leak the program.
		throw std::runtime_error(str);
	}
	// Always detach shaders after a successful link.
	if (info.vertex != 0)
	{
		glDetachShader(this->id, info.vertex);
		glDeleteShader(info.vertex);
	}
	if (info.frag != 0)
	{
		glDetachShader(this->id, info.frag);
		glDeleteShader(info.frag);
	}
	if (info.compute != 0)
	{
		glDetachShader(this->id, info.compute);
		glDeleteShader(info.compute);
	}

	for (Uniform uniform : info.uniforms)
		uniforms.insert(std::make_pair(uniform.name, glGetUniformLocation(this->id, uniform.name.c_str())));

	glValidateProgram(this->id);
}
void Shader::destroy()
{
	glDeleteProgram(this->id);
}

void Shader::use()
{
	glUseProgram(this->id);
}

GLint Shader::getUniformLocation(const char* name)
{
	auto it = uniforms.find(name);
	if (it == uniforms.end())
		throw std::runtime_error("Uniform not found : " + std::string(name));
	return it->second;
}

void Shader::setFloat1(const char* name, float value)
{
	glUniform1f(getUniformLocation(name), value);
}

void Shader::setFloat2(const char* name, const vec2& value)
{
	glUniform2f(getUniformLocation(name), value.x, value.y);
}

void Shader::setFloat3(const char* name, const vec3& value)
{
	glUniform3f(getUniformLocation(name), value.x, value.y, value.z);
}

void Shader::setFloat4(const char* name, const vec4& value)
{
	glUniform4f(getUniformLocation(name), value.x, value.y, value.z, value.w);
}

void Shader::setMat4(const char* name, const mat4& value)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value.data);
}

ShaderID Shader::create(const char* content, ShaderType type)
{
	GLuint shaderID = glCreateShader(Shader::getType(type));
	glShaderSource(shaderID, 1, &content, NULL);
	glCompileShader(shaderID);
	GLint isCompiled;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);
		std::string str(errorLog.begin(), errorLog.end());
		// Exit with failure.
		glDeleteShader(shaderID); // Don't leak the shader.
		std::cerr << str << std::endl;
		throw std::runtime_error(str);
	}
	return shaderID;
}

GLuint Shader::getType(ShaderType type)
{
	switch (type)
	{
	case ShaderType::VERTEX_SHADER:
		return GL_VERTEX_SHADER;
	case ShaderType::TESS_CONTROL_SHADER:
		return GL_TESS_CONTROL_SHADER;
	case ShaderType::TESS_EVALUATION_SHADER:
		return GL_TESS_EVALUATION_SHADER;
	case ShaderType::FRAGMENT_SHADER:
		return GL_FRAGMENT_SHADER;
	case ShaderType::GEOMETRY_SHADER:
		return GL_GEOMETRY_SHADER;
	case ShaderType::COMPUTE_SHADER:
		return GL_COMPUTE_SHADER;
	default:
		return 0;
	}
}

};
};