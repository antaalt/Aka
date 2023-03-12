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

enum class ShaderBindingType : uint8_t
{
	None,
	SampledImage,
	StorageImage,
	UniformBuffer,
	StorageBuffer,
	AccelerationStructure,
};

struct ShaderBindingLayout
{
	ShaderBindingType type; // Type of binding
	ShaderMask stages; // Shader stages used by this binding
	uint32_t count; // Number of element for this binding
};

struct ShaderBindingState
{
	ShaderBindingLayout bindings[ShaderMaxBindingCount];
	uint32_t count;

	ShaderBindingState& add(ShaderBindingType type, ShaderMask stages, uint32_t bindingCount = 1)
	{
		AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
		bindings[count++] = ShaderBindingLayout{ type, stages, bindingCount };
		return *this;
	}
};

struct Program : Resource
{
	Program(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* sets, uint32_t bindingCounts);
	Program(const char* name, ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* sets, uint32_t bindingCounts);
	Program(const char* name, ShaderHandle compute, const ShaderBindingState* sets, uint32_t bindingCounts);

	// TODO replace by ShaderType flag as we could delete shader after program creation.
	ShaderHandle vertex;
	ShaderHandle fragment;
	ShaderHandle geometry;
	ShaderHandle compute;

	ShaderBindingState sets[ShaderMaxSetCount];

	uint32_t setCount;

	bool hasVertexStage() const;
	bool hasFragmentStage() const;
	bool hasGeometryStage() const;
	bool hasComputeStage() const;

	static ProgramHandle createVertex(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* bindings, uint32_t bindingCounts);
	static ProgramHandle createGeometry(const char* name, ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* bindings, uint32_t bindingCounts);
	static ProgramHandle createCompute(const char* name, ShaderHandle compute, const ShaderBindingState* bindings, uint32_t count);
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
		aka::hashCombine(hash, data.count);
		for (size_t i = 0; i < data.count; i++)
		{
			aka::hashCombine(hash, data.bindings[i].count);
			aka::hashCombine(hash, aka::EnumToIntegral(data.bindings[i].stages));
			aka::hashCombine(hash, aka::EnumToIntegral(data.bindings[i].type));
		}
		return hash;
	}
};