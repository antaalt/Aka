#include "Shader.h"

#include <stdexcept>
#include <vector>

#include "GLBackend.h"

namespace aka {

Shader::Shader() :
	m_programID(0)
{

}
Shader::~Shader()
{
	if (m_programID != 0)
		destroy();
}

void Shader::create(const ShaderInfo& info)
{
	m_programID = glCreateProgram();
	// Attach shaders
	if (info.vertex != 0 && glIsShader(info.vertex) == GL_TRUE)
		glAttachShader(m_programID, info.vertex);
	if (info.frag != 0 && glIsShader(info.frag) == GL_TRUE)
		glAttachShader(m_programID, info.frag);
	if (info.compute != 0 && glIsShader(info.compute) == GL_TRUE)
		glAttachShader(m_programID, info.compute);

	// link program
	glLinkProgram(m_programID);
	GLint linked;
	glGetProgramiv(m_programID, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(m_programID, maxLength, &maxLength, &errorLog[0]);
		std::string str(errorLog.begin(), errorLog.end());
		// Exit with failure.
		glDeleteProgram(m_programID); // Don't leak the program.
		throw std::runtime_error(str);
	}
	// Always detach shaders after a successful link.
	if (info.vertex != 0)
	{
		glDetachShader(m_programID, info.vertex);
		glDeleteShader(info.vertex);
	}
	if (info.frag != 0)
	{
		glDetachShader(m_programID, info.frag);
		glDeleteShader(info.frag);
	}
	if (info.compute != 0)
	{
		glDetachShader(m_programID, info.compute);
		glDeleteShader(info.compute);
	}

	for (Uniform uniform : info.uniforms)
		m_uniforms.insert(std::make_pair(uniform.name, glGetUniformLocation(m_programID, uniform.name.c_str())));

	glValidateProgram(m_programID);
}
void Shader::destroy()
{
	glDeleteProgram(m_programID);
}

void Shader::use()
{
	glUseProgram(m_programID);
}

UniformID Shader::getUniformLocation(const char* name)
{
	auto it = m_uniforms.find(name);
	if (it == m_uniforms.end())
		throw std::runtime_error("Uniform not found : " + std::string(name));
	return it->second;
}

void Shader::setFloat1(const char* name, float value)
{
	glUniform1f(getUniformLocation(name), value);
}

void Shader::setFloat2(const char* name, float x, float y)
{
	glUniform2f(getUniformLocation(name), x, y);
}

void Shader::setFloat3(const char* name, float x, float y, float z)
{
	glUniform3f(getUniformLocation(name), x, y, z);
}

void Shader::setFloat4(const char* name, float x, float y, float z, float w)
{
	glUniform4f(getUniformLocation(name), x, y, z, w);
}

void Shader::setUint1(const char* name, uint32_t value)
{
	glUniform1ui(getUniformLocation(name), value);
}

void Shader::setUint2(const char* name, uint32_t x, uint32_t y)
{
	glUniform2ui(getUniformLocation(name), x, y);
}

void Shader::setUint3(const char* name, uint32_t x, uint32_t y, uint32_t z)
{
	glUniform3ui(getUniformLocation(name), x, y, z);
}

void Shader::setUint4(const char* name, uint32_t x, uint32_t y, uint32_t z, uint32_t w)
{
	glUniform4ui(getUniformLocation(name), x, y, z, w);
}

void Shader::setInt1(const char* name, int32_t value)
{
	glUniform1i(getUniformLocation(name), value);
}

void Shader::setInt2(const char* name, int32_t x, int32_t y)
{
	glUniform2i(getUniformLocation(name), x, y);
}

void Shader::setInt3(const char* name, int32_t x, int32_t y, int32_t z)
{
	glUniform3i(getUniformLocation(name), x, y, z);
}

void Shader::setInt4(const char* name, int32_t x, int32_t y, int32_t z, int32_t w)
{
	glUniform4i(getUniformLocation(name), x, y, z, w);
}

void Shader::setMatrix4(const char* name, const float* data, bool transpose)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, transpose, data);
}

GLuint getType(ShaderType type)
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

ShaderID Shader::create(const std::string& content, ShaderType type)
{
	return Shader::create(content.c_str(), type);
}

ShaderID Shader::create(const char* content, ShaderType type)
{
	GLuint shaderID = glCreateShader(getType(type));
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


template <>
void Shader::set(const char* name, float value) {
	setFloat1(name, value);
}
template <>
void Shader::set(const char* name, vec2f value) {
	setFloat2(name, value.x, value.y);
}
template <>
void Shader::set(const char* name, vec3f value) {
	setFloat3(name, value.x, value.y, value.z);
}
template <>
void Shader::set(const char* name, vec4f value) {
	setFloat4(name, value.x, value.y, value.z, value.w);
}
template <>
void Shader::set(const char* name, uint32_t value) {
	setUint1(name, value);
}
template <>
void Shader::set(const char* name, vec2u value) {
	setUint2(name, value.x, value.y);
}
template <>
void Shader::set(const char* name, int32_t value) {
	setInt1(name, value);
}
template <>
void Shader::set(const char* name, vec2i value) {
	setInt2(name, value.x, value.y);
}
template <>
void Shader::set(const char* name, color4f value) {
	setFloat4(name, value.r, value.g, value.b, value.a);
}
template <>
void Shader::set(const char* name, mat4f value) {
	setMatrix4(name, &value.cols[0].x, false);
}

};