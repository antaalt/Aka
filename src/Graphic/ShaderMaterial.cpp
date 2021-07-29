#include <Aka/Graphic/ShaderMaterial.h>

#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/OS/Logger.h>

#include <cstring>

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

Shader::Ptr ShaderMaterial::getShader()
{
	return m_shader;
}

float* ShaderMaterial::findUniformOffset(const char* name)
{
	float* offset = m_data.data();
	for (const Uniform& uniform : m_uniforms)
	{
		switch (uniform.type)
		{
		case UniformType::None:
		case UniformType::Texture2D:
		case UniformType::TextureCubemap:
		case UniformType::Sampler2D:
		case UniformType::SamplerCube:
		case UniformType::Image2D:
			continue;
		case UniformType::Mat4:
			if (uniform.name == std::string(name))
				return offset;
			offset += 16 * uniform.arrayLength;
			break;
		case UniformType::Mat3:
			if (uniform.name == std::string(name))
				return offset;
			offset += 9 * uniform.arrayLength;
			break;
		case UniformType::Vec2:
			if (uniform.name == std::string(name))
				return offset;
			offset += 2 * uniform.arrayLength;
			break;
		case UniformType::Vec3:
			if (uniform.name == std::string(name))
				return offset;
			offset += 3 * uniform.arrayLength;
			break;
		case UniformType::Vec4:
			if (uniform.name == std::string(name))
				return offset;
			offset += 4 * uniform.arrayLength;
			break;
		case UniformType::Float:
		case UniformType::Int:
		case UniformType::UnsignedInt:
			if (uniform.name == std::string(name))
				return offset;
			offset += 1 * uniform.arrayLength;
			break;
		default:
			Logger::error("Unsupported uniform type : ", (int)uniform.type);
			break;
		}
	}
	Logger::error("Uniform not found : ", name);
	return nullptr;
}
void ShaderMaterial::setFloat1(const char* name, const float* value, size_t count)
{
	// TODO assert type is correct.
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(float) * count);
}
void ShaderMaterial::setFloat2(const char* name, const float* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(float) * 2 * count);
}
void ShaderMaterial::setFloat3(const char* name, const float* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(float) * 3 * count);
}
void ShaderMaterial::setFloat4(const char* name, const float* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(float) * 4 * count);
}
void ShaderMaterial::setUint1(const char* name, const uint32_t* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(uint32_t) * count);
}
void ShaderMaterial::setUint2(const char* name, const uint32_t* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(uint32_t) * 2 * count);
}
void ShaderMaterial::setUint3(const char* name, const uint32_t* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(uint32_t) * 3 * count);
}
void ShaderMaterial::setUint4(const char* name, const uint32_t* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(uint32_t) * 4 * count);
}
void ShaderMaterial::setInt1(const char* name, const int32_t* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(int32_t) * count);
}
void ShaderMaterial::setInt2(const char* name, const int32_t* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(int32_t) * 2 * count);
}
void ShaderMaterial::setInt3(const char* name, const int32_t* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(int32_t) * 3 * count);
}
void ShaderMaterial::setInt4(const char* name, const int32_t* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, sizeof(int32_t) * 4 * count);
}
void ShaderMaterial::setMatrix3(const char* name, const float* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, 9 * sizeof(float) * count);
}
void ShaderMaterial::setMatrix4(const char* name, const float* value, size_t count)
{
	float* offset = findUniformOffset(name);
	if (offset == nullptr) return;
	memcpy(offset, value, 16 * sizeof(float) * count);
}
void ShaderMaterial::setTexture(const char* name, const Texture::Ptr* textures, size_t count)
{
	uint32_t slot = 0;
	for (const Uniform& uniform : m_uniforms)
	{
		if (uniform.type != UniformType::Texture2D && uniform.type != UniformType::TextureCubemap)
			continue;
		if (uniform.name == std::string(name))
		{
			for (size_t iTex = 0; iTex < count; iTex++)
				m_textures[slot + iTex] = textures[iTex];
			return;
		}
		slot += 1 * uniform.arrayLength;
	}
	Logger::error("Texture not found : ", name);
}
void ShaderMaterial::setImage(const char* name, const Texture::Ptr* images, size_t count)
{
	uint32_t slot = 0;
	for (const Uniform& uniform : m_uniforms)
	{
		if (uniform.type != UniformType::Image2D)
			continue;
		if (uniform.name == std::string(name))
		{
			for (size_t iTex = 0; iTex < count; iTex++)
				m_images[slot + iTex] = images[iTex];
			return;
		}
		slot += 1 * uniform.arrayLength;
	}
	Logger::error("Image not found : ", name);
}

template <>
void ShaderMaterial::set(const char* name, float value) {
	setFloat1(name, &value, 1);
}
template <>
void ShaderMaterial::set(const char* name, vec2f value) {
	setFloat2(name, value.data, 1);
}
template <>
void ShaderMaterial::set(const char* name, vec3f value) {
	setFloat3(name, value.data, 1);
}
template <>
void ShaderMaterial::set(const char* name, vec4f value) {
	setFloat4(name, value.data, 1);
}
template <>
void ShaderMaterial::set(const char* name, uint32_t value) {
	setUint1(name, &value, 1);
}
template <>
void ShaderMaterial::set(const char* name, vec2u value) {
	setUint2(name, value.data, 1);
}
template <>
void ShaderMaterial::set(const char* name, int32_t value) {
	setInt1(name, &value, 1);
}
template <>
void ShaderMaterial::set(const char* name, vec2i value) {
	setInt2(name, value.data, 1);
}
template <>
void ShaderMaterial::set(const char* name, color3f value) {
	setFloat3(name, value.data, 1);
}
template <>
void ShaderMaterial::set(const char* name, color4f value) {
	setFloat4(name, value.data, 1);
}
template <>
void ShaderMaterial::set(const char* name, mat3f value) {
	setMatrix3(name, value.cols[0].data, 1);
}
template <>
void ShaderMaterial::set(const char* name, mat4f value) {
	setMatrix4(name, value.cols[0].data, 1);
}
template <>
void ShaderMaterial::set(const char* name, Texture::Ptr texture) {
	setTexture(name, &texture, 1);
}
template <>
void ShaderMaterial::set(const char* name, const float* value, size_t count) {
	setFloat1(name, value, count);
}
template <>
void ShaderMaterial::set(const char* name, const mat4f* value, size_t count) {
	setMatrix4(name, value[0].cols[0].data, count);
}
template <>
void ShaderMaterial::set(const char* name, const Texture::Ptr* value, size_t count) {
	setTexture(name, value, count);
}

};