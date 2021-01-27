#include "Shader.h"

#include "GraphicBackend.h"
#include "../Platform/Logger.h"

namespace aka {

Shader::Shader(const std::vector<Attributes>& attributes) :
	m_attributes(attributes)
{
}

Shader::~Shader()
{
}

ShaderID Shader::compile(const std::string& content, ShaderType type)
{
	return GraphicBackend::compile(content.c_str(), type);
}

ShaderID Shader::compile(const char* content, ShaderType type)
{
	return GraphicBackend::compile(content, type);
}

UniformID Shader::getUniformID(const char* name)
{
	for (Uniform uniform : m_uniforms)
	{
		if (uniform.name == std::string(name))
			return uniform.id;
	}
	Logger::error("Uniform not found : ", name);
	return UniformID(0);
}

const Uniform *Shader::getUniform(const char* name) const
{
	for (const Uniform &uniform : m_uniforms)
	{
		if (uniform.name == std::string(name))
			return &uniform;
	}
	Logger::error("Uniform not found : ", name);
	return nullptr;
}

Shader::Ptr Shader::create(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes)
{
	return GraphicBackend::createShader(vert, frag, compute, attributes);
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