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

struct ShaderBindingState
{
	ShaderBindingLayout bindings[ShaderMaxBindingCount];
	uint32_t count;

	ShaderBindingState& add(ShaderBindingType type, ShaderMask stages, ShaderBindingFlag flags = ShaderBindingFlag::None, uint32_t bindingCount = 1)
	{
		AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
		bindings[count++] = ShaderBindingLayout{ type, stages,flags, bindingCount };
		return *this;
	}
};

struct ShaderConstant
{
	uint32_t offset;
	uint32_t size;
	ShaderMask shader;
};

struct Program : Resource
{
	Program(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount);
	Program(const char* name, ShaderHandle task, ShaderHandle mesh, ShaderHandle fragment, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount);
	Program(const char* name, ShaderHandle compute, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount);

	// TODO replace by ShaderType flag as we could delete shader after program creation.
	ShaderHandle task;
	ShaderHandle mesh;
	ShaderHandle vertex;
	ShaderHandle fragment;
	ShaderHandle compute;

	ShaderBindingState sets[ShaderMaxSetCount];

	ShaderConstant constants[ShaderMaxConstantCount];

	uint32_t setCount;
	uint32_t constantCount;

	bool hasMeshStage() const;
	bool hasTaskStage() const;
	bool hasVertexStage() const;
	bool hasFragmentStage() const;
	bool hasComputeStage() const;

	static ProgramHandle createVertex(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* bindings, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount);
	static ProgramHandle createMesh(const char* name, ShaderHandle task, ShaderHandle mesh, ShaderHandle fragment, const ShaderBindingState* bindings, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount);
	static ProgramHandle createCompute(const char* name, ShaderHandle compute, const ShaderBindingState* bindings, uint32_t count, const ShaderConstant* constants, uint32_t constantCount);
	static void destroy(ProgramHandle program);
};

bool operator<(const ShaderBindingState& lhs, const ShaderBindingState& rhs);
bool operator>(const ShaderBindingState& lhs, const ShaderBindingState& rhs);
bool operator==(const ShaderBindingState& lhs, const ShaderBindingState& rhs);
bool operator!=(const ShaderBindingState& lhs, const ShaderBindingState& rhs);

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