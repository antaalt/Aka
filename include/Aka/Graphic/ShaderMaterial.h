#pragma once

#include "Shader.h"

namespace aka {

class ShaderMaterial
{
public:
	using Ptr = std::shared_ptr<ShaderMaterial>;
protected:
	ShaderMaterial(Shader::Ptr shader);
	ShaderMaterial(const ShaderMaterial&) = delete;
	const ShaderMaterial& operator=(const ShaderMaterial&) = delete;
	virtual ~ShaderMaterial();
public:
	static ShaderMaterial::Ptr create(Shader::Ptr shader);

	template <typename T>
	void set(const char* name, T value);

	Shader::Ptr getShader();

protected:
	float* findUniformOffset(const char* name);
	UniformID getUniformID(const char* name);
	const Uniform* getUniform(const char* name) const;
	void setFloat1(const char* name, float value);
	void setFloat2(const char* name, float x, float y);
	void setFloat3(const char* name, float x, float y, float z);
	void setFloat4(const char* name, float x, float y, float z, float w);
	void setUint1(const char* name, uint32_t value);
	void setUint2(const char* name, uint32_t x, uint32_t y);
	void setUint3(const char* name, uint32_t x, uint32_t y, uint32_t z);
	void setUint4(const char* name, uint32_t x, uint32_t y, uint32_t z, uint32_t w);
	void setInt1(const char* name, int32_t value);
	void setInt2(const char* name, int32_t x, int32_t y);
	void setInt3(const char* name, int32_t x, int32_t y, int32_t z);
	void setInt4(const char* name, int32_t x, int32_t y, int32_t z, int32_t w);
	void setMatrix3(const char* name, const float* data, bool transpose = false);
	void setMatrix4(const char* name, const float* data, bool transpose = false);
	void setTexture(const char* name, Texture::Ptr texture);
protected:
	Shader::Ptr m_shader;
	std::vector<float> m_data;
	std::vector<Uniform> m_uniforms;
	std::vector<Texture::Ptr> m_textures;
};

}