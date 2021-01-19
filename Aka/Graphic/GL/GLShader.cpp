#include "../Shader.h"

#include <stdexcept>
#include <vector>

#include "GLBackend.h"

namespace aka {

Shader::Shader() :
	m_programID(0)
{
}

Shader::Shader(ShaderID vertex, ShaderID frag, ShaderID compute, const std::vector<Uniform>& uniforms)
{
	m_programID = ProgramID(glCreateProgram());
	// Attach shaders
	if (vertex() != 0 && glIsShader(vertex()) == GL_TRUE)
		glAttachShader(m_programID(), vertex());
	if (frag() != 0 && glIsShader(frag()) == GL_TRUE)
		glAttachShader(m_programID(), frag());
	if (compute() != 0 && glIsShader(compute()) == GL_TRUE)
		glAttachShader(m_programID(), compute());

	// link program
	glLinkProgram(m_programID());
	GLint linked;
	glGetProgramiv(m_programID(), GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(m_programID(), GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(m_programID(), maxLength, &maxLength, &errorLog[0]);
		std::string str(errorLog.begin(), errorLog.end());
		// Exit with failure.
		glDeleteProgram(m_programID()); // Don't leak the program.
		throw std::runtime_error(str);
	}
	// Always detach shaders after a successful link.
	if (vertex() != 0)
	{
		glDetachShader(m_programID(), vertex());
		glDeleteShader(vertex());
	}
	if (frag() != 0)
	{
		glDetachShader(m_programID(), frag());
		glDeleteShader(frag());
	}
	if (compute() != 0)
	{
		glDetachShader(m_programID(), compute());
		glDeleteShader(compute());
	}

	for (Uniform uniform : uniforms)
		m_uniforms.insert(std::make_pair(uniform.name, glGetUniformLocation(m_programID(), uniform.name.c_str())));

	glValidateProgram(m_programID());
}

Shader::~Shader()
{
	if (m_programID() != 0)
		glDeleteProgram(m_programID());
}

void Shader::use()
{
	glUseProgram(m_programID());
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
	glUniform1f(getUniformLocation(name).value(), value);
}

void Shader::setFloat2(const char* name, float x, float y)
{
	glUniform2f(getUniformLocation(name).value(), x, y);
}

void Shader::setFloat3(const char* name, float x, float y, float z)
{
	glUniform3f(getUniformLocation(name).value(), x, y, z);
}

void Shader::setFloat4(const char* name, float x, float y, float z, float w)
{
	glUniform4f(getUniformLocation(name).value(), x, y, z, w);
}

void Shader::setUint1(const char* name, uint32_t value)
{
	glUniform1ui(getUniformLocation(name).value(), value);
}

void Shader::setUint2(const char* name, uint32_t x, uint32_t y)
{
	glUniform2ui(getUniformLocation(name).value(), x, y);
}

void Shader::setUint3(const char* name, uint32_t x, uint32_t y, uint32_t z)
{
	glUniform3ui(getUniformLocation(name).value(), x, y, z);
}

void Shader::setUint4(const char* name, uint32_t x, uint32_t y, uint32_t z, uint32_t w)
{
	glUniform4ui(getUniformLocation(name).value(), x, y, z, w);
}

void Shader::setInt1(const char* name, int32_t value)
{
	glUniform1i(getUniformLocation(name).value(), value);
}

void Shader::setInt2(const char* name, int32_t x, int32_t y)
{
	glUniform2i(getUniformLocation(name).value(), x, y);
}

void Shader::setInt3(const char* name, int32_t x, int32_t y, int32_t z)
{
	glUniform3i(getUniformLocation(name).value(), x, y, z);
}

void Shader::setInt4(const char* name, int32_t x, int32_t y, int32_t z, int32_t w)
{
	glUniform4i(getUniformLocation(name).value(), x, y, z, w);
}

void Shader::setMatrix4(const char* name, const float* data, bool transpose)
{
	glUniformMatrix4fv(getUniformLocation(name).value(), 1, transpose, data);
}

GLuint getType(ShaderType type)
{
	switch (type)
	{
	case ShaderType::Vertex:
		return GL_VERTEX_SHADER;
	case ShaderType::TesselationControl:
		return GL_TESS_CONTROL_SHADER;
	case ShaderType::TesselationEvaluation:
		return GL_TESS_EVALUATION_SHADER;
	case ShaderType::Fragment:
		return GL_FRAGMENT_SHADER;
	case ShaderType::Geometry:
		return GL_GEOMETRY_SHADER;
	case ShaderType::Compute:
		return GL_COMPUTE_SHADER;
	default:
		return 0;
	}
}

ShaderID Shader::compile(const std::string& content, ShaderType type)
{
	return Shader::compile(content.c_str(), type);
}

ShaderID Shader::compile(const char* content, ShaderType type)
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
	return ShaderID(shaderID);
}

Shader::Ptr Shader::create(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Uniform>& uniforms)
{
	return std::make_shared<Shader>(vert, frag, compute, uniforms);
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