#pragma once

#include <stdint.h>

#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

struct Program;
using ProgramHandle = ResourceHandle<Program>;

static constexpr uint32_t ShaderMaxBindingCount = 16; // TODO increase / Retrieve from API
static constexpr uint32_t ShaderMaxSetCount = 8;
static constexpr uint32_t ShaderMaxConstantCount = 8;

enum class ShaderBindingType : uint8_t
{
	None,
	SampledImage,
	StorageImage,
	UniformBuffer,
	StorageBuffer,
	AccelerationStructure,
};

enum class ShaderBindingFlag
{
	None			= 0,
	Bindless		= 1 << 0,
	UpdateAfterBind = 1 << 1,
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ShaderBindingFlag);

struct ShaderBindingLayout
{
	ShaderBindingType type = ShaderBindingType::None; // Type of binding
	ShaderMask stages = ShaderMask::None; // Shader stages used by this binding
	ShaderBindingFlag flags = ShaderBindingFlag::None;
	uint32_t count = 1; // Number of element for this binding
};
// set hold descriptor hold sets.
struct ShaderBindingState
{
	ShaderBindingLayout bindings[ShaderMaxBindingCount] = {};
	uint32_t count = 0;

	ShaderBindingState& add(ShaderBindingType type, ShaderMask stages, ShaderBindingFlag flags = ShaderBindingFlag::None, uint32_t bindingCount = 1)
	{
		AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
		bindings[count++] = ShaderBindingLayout{ type, stages,flags, bindingCount };
		return *this;
	}
};

struct ShaderConstant
{
	uint32_t offset = 0;
	uint32_t size = 0;
	ShaderMask shader = ShaderMask::None;
};

struct ShaderPipelineLayout 
{
	ShaderBindingState sets[ShaderMaxSetCount] = {};
	uint32_t setCount = 0;
	ShaderConstant constants[ShaderMaxConstantCount] = {};
	uint32_t constantCount = 0;


	ShaderPipelineLayout& addSet(const ShaderBindingState& set)
	{
		AKA_ASSERT(setCount + 1 < ShaderMaxSetCount, "Too many shader sets");
		sets[setCount++] = set;
		return *this;
	}
	ShaderPipelineLayout& addConstant(const ShaderConstant& constant)
	{
		AKA_ASSERT(constantCount + 1 < ShaderMaxConstantCount, "Too many shader constants");
		constants[constantCount++] = constant;
		return *this;
	}

	bool isValid() const;
};

struct Program : Resource
{
	Program(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderPipelineLayout& layout);
	Program(const char* name, ShaderHandle task, ShaderHandle mesh, ShaderHandle fragment, const ShaderPipelineLayout& layout);
	Program(const char* name, ShaderHandle compute, const ShaderPipelineLayout& layout);
	virtual ~Program() {}

	ShaderHandle shaders[EnumCount<ShaderType>()];

	bool hasShaderStage(ShaderType _shaderType) const;
	bool hasMeshStage() const;
	bool hasTaskStage() const;
	bool hasVertexStage() const;
	bool hasFragmentStage() const;
	bool hasComputeStage() const;

	bool isCompatible(const ShaderPipelineLayout& _layout) const;
private:
	// Reflection data. Do not expose.
	ShaderPipelineLayout layout;
};

bool operator<(const ShaderBindingState& lhs, const ShaderBindingState& rhs);
bool operator>(const ShaderBindingState& lhs, const ShaderBindingState& rhs);
bool operator==(const ShaderBindingState& lhs, const ShaderBindingState& rhs);
bool operator!=(const ShaderBindingState& lhs, const ShaderBindingState& rhs);

bool operator<(const ShaderConstant& lhs, const ShaderConstant& rhs);
bool operator>(const ShaderConstant& lhs, const ShaderConstant& rhs);
bool operator==(const ShaderConstant& lhs, const ShaderConstant& rhs);
bool operator!=(const ShaderConstant& lhs, const ShaderConstant& rhs);

bool operator<(const ShaderPipelineLayout& lhs, const ShaderPipelineLayout& rhs);
bool operator>(const ShaderPipelineLayout& lhs, const ShaderPipelineLayout& rhs);
bool operator==(const ShaderPipelineLayout& lhs, const ShaderPipelineLayout& rhs);
bool operator!=(const ShaderPipelineLayout& lhs, const ShaderPipelineLayout& rhs);

};
};

template<> struct std::hash<aka::gfx::ShaderBindingState>
{
	size_t operator()(const aka::gfx::ShaderBindingState& data) const
	{
		size_t hash = 0;
		aka::hash::combine(hash, data.count);
		for (size_t i = 0; i < data.count; i++)
		{
			aka::hash::combine(hash, data.bindings[i].count);
			aka::hash::combine(hash, data.bindings[i].flags);
			aka::hash::combine(hash, static_cast<uint32_t>(data.bindings[i].stages));
			aka::hash::combine(hash, static_cast<uint32_t>(data.bindings[i].type));
		}
		return hash;
	}
};