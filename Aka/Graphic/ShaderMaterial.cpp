#include "ShaderMaterial.h"

#include "GraphicBackend.h"
#include "../OS/Logger.h"

namespace aka {

ShaderMaterial::ShaderMaterial(Shader::Ptr shader) :
	m_shader(shader)
{
}

ShaderMaterial::~ShaderMaterial()
{
}

ShaderMaterial::Ptr ShaderMaterial::create(Shader::Ptr shader)
{
	return GraphicBackend::createShaderMaterial(shader);
}

UniformID ShaderMaterial::getUniformID(const char* name)
{
	for (const Uniform &uniform : m_uniforms)
	{
		if (uniform.name == std::string(name))
			return uniform.id;
	}
	Logger::error("Uniform not found : ", name);
	return UniformID(0);
}

const Uniform* ShaderMaterial::getUniform(const char* name) const
{
	for (const Uniform& uniform : m_uniforms)
	{
		if (uniform.name == std::string(name))
			return &uniform;
	}
	Logger::error("Uniform not found : ", name);
	return nullptr;
}

float* ShaderMaterial::findUniformOffset(const char* name)
{
	float* offset = m_data.data();
	for (const Uniform& uniform : m_uniforms)
	{
		if (uniform.type == UniformType::None || uniform.type == UniformType::Texture2D || uniform.type == UniformType::Sampler2D)
			continue;
		if (uniform.type == UniformType::Mat4)
		{
			if (uniform.name == std::string(name))
				return offset;
			offset += 16 * uniform.arrayLength;
		}
		else if (uniform.type == UniformType::Mat3)
		{
			if (uniform.name == std::string(name))
				return offset;
			offset += 9 * uniform.arrayLength;
		}
		else if (uniform.type == UniformType::Vec4)
		{
			if (uniform.name == std::string(name))
				return offset;
			offset += 4 * uniform.arrayLength;
		}
		else
		{
			Logger::error("Unsupported uniform type : ", (int)uniform.type);
		}
	}
	Logger::error("Uniform not found : ", name);
	return nullptr;
}
void ShaderMaterial::setFloat1(const char* name, float value)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, &value, sizeof(float));
}
void ShaderMaterial::setFloat2(const char* name, float x, float y)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	float data[] = { x,y };
	memcpy(offset, data, 2 * sizeof(float));
}
void ShaderMaterial::setFloat3(const char* name, float x, float y, float z)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	float data[] = { x,y,z };
	memcpy(offset, data, 3 * sizeof(float));
}
void ShaderMaterial::setFloat4(const char* name, float x, float y, float z, float w)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	float data[] = { x,y,z,w };
	memcpy(offset, data, 4 * sizeof(float));
}
void ShaderMaterial::setUint1(const char* name, uint32_t value)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, &value, sizeof(float));
}
void ShaderMaterial::setUint2(const char* name, uint32_t x, uint32_t y)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	uint32_t data[] = { x,y };
	memcpy(offset, data, 2 * sizeof(float));
}
void ShaderMaterial::setUint3(const char* name, uint32_t x, uint32_t y, uint32_t z)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	uint32_t data[] = { x,y,z };
	memcpy(offset, data, 3 * sizeof(float));
}
void ShaderMaterial::setUint4(const char* name, uint32_t x, uint32_t y, uint32_t z, uint32_t w)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	uint32_t data[] = { x,y,z,w };
	memcpy(offset, data, 4 * sizeof(float));
}
void ShaderMaterial::setInt1(const char* name, int32_t value)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, &value, sizeof(float));
}
void ShaderMaterial::setInt2(const char* name, int32_t x, int32_t y)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	int32_t data[] = { x,y };
	memcpy(offset, data, 2 * sizeof(float));
}
void ShaderMaterial::setInt3(const char* name, int32_t x, int32_t y, int32_t z)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	int32_t data[] = { x,y,z };
	memcpy(offset, data, 3 * sizeof(float));
}
void ShaderMaterial::setInt4(const char* name, int32_t x, int32_t y, int32_t z, int32_t w)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	int32_t data[] = { x,y,z,w };
	memcpy(offset, data, 4 * sizeof(float));
}
void ShaderMaterial::setMatrix3(const char* name, const float* data, bool transpose)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, data, 9 * sizeof(float));
}
void ShaderMaterial::setMatrix4(const char* name, const float* data, bool transpose)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, data, 16 * sizeof(float));
}
void ShaderMaterial::setTexture(const char* name, Texture::Ptr texture)
{
	uint32_t slot = 0;
	for (const Uniform &uniform : m_uniforms)
	{
		if (uniform.type != UniformType::Texture2D)
			continue;
		if (uniform.name == std::string(name))
		{
			m_textures[slot] = texture;
			return;
		}
		slot++;
	}
	Logger::error("Texture not found : ", name);
}

template <>
void ShaderMaterial::set(const char* name, float value) {
	setFloat1(name, value);
}
template <>
void ShaderMaterial::set(const char* name, vec2f value) {
	setFloat2(name, value.x, value.y);
}
template <>
void ShaderMaterial::set(const char* name, vec3f value) {
	setFloat3(name, value.x, value.y, value.z);
}
template <>
void ShaderMaterial::set(const char* name, vec4f value) {
	setFloat4(name, value.x, value.y, value.z, value.w);
}
template <>
void ShaderMaterial::set(const char* name, uint32_t value) {
	setUint1(name, value);
}
template <>
void ShaderMaterial::set(const char* name, vec2u value) {
	setUint2(name, value.x, value.y);
}
template <>
void ShaderMaterial::set(const char* name, int32_t value) {
	setInt1(name, value);
}
template <>
void ShaderMaterial::set(const char* name, vec2i value) {
	setInt2(name, value.x, value.y);
}
template <>
void ShaderMaterial::set(const char* name, color4f value) {
	setFloat4(name, value.r, value.g, value.b, value.a);
}
template <>
void ShaderMaterial::set(const char* name, mat3f value) {
	setMatrix3(name, &value.cols[0].x, false);
}
template <>
void ShaderMaterial::set(const char* name, mat4f value) {
	setMatrix4(name, &value.cols[0].x, false);
}
template <>
void ShaderMaterial::set(const char* name, Texture::Ptr texture) {
	setTexture(name, texture);
}

};