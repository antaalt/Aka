#include <Aka/Graphic/Program.h>

#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

Program::Program(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* sets, uint32_t bindingCounts) :
	Resource(name, ResourceType::Program),
	vertex(vertex),
	fragment(fragment),
	geometry(ShaderHandle::null),
	compute(ShaderHandle::null),
	sets{},
	setCount(bindingCounts)
{
	AKA_ASSERT(bindingCounts < ShaderMaxBindingCount, "Too much bindings");
	memcpy(this->sets, sets, bindingCounts * sizeof(ShaderBindingState));
}

Program::Program(const char* name, ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* sets, uint32_t bindingCounts) :
	Resource(name, ResourceType::Program),
	vertex(vertex),
	fragment(fragment),
	geometry(geometry),
	compute(ShaderHandle::null),
	sets{},
	setCount(bindingCounts)
{
	AKA_ASSERT(bindingCounts < ShaderMaxBindingCount, "Too much bindings");
	memcpy(this->sets, sets, bindingCounts * sizeof(ShaderBindingState));
}
Program::Program(const char* name, ShaderHandle compute, const ShaderBindingState* sets, uint32_t bindingCounts) :
	Resource(name, ResourceType::Program),
	vertex(ShaderHandle::null),
	fragment(ShaderHandle::null),
	geometry(ShaderHandle::null),
	compute(compute),
	sets{},
	setCount(bindingCounts)
{
	AKA_ASSERT(bindingCounts < ShaderMaxBindingCount, "Too much bindings");
	memcpy(this->sets, sets, bindingCounts * sizeof(ShaderBindingState));
}

bool operator<(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	if (lhs.count < rhs.count) return true;
	else if (lhs.count > rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.bindings[i].count < rhs.bindings[i].count) return true;
		else if (lhs.bindings[i].count > rhs.bindings[i].count) return false;
		if (lhs.bindings[i].stages < rhs.bindings[i].stages) return true;
		else if (lhs.bindings[i].stages > rhs.bindings[i].stages) return false;
		if (lhs.bindings[i].type < rhs.bindings[i].type) return true;
		else if (lhs.bindings[i].type > rhs.bindings[i].type) return false;
	}
	return false; // equal
}
bool operator>(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	if (lhs.count > rhs.count) return true;
	else if (lhs.count < rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.bindings[i].count > rhs.bindings[i].count) return true;
		else if (lhs.bindings[i].count < rhs.bindings[i].count) return false;
		if (lhs.bindings[i].stages > rhs.bindings[i].stages) return true;
		else if (lhs.bindings[i].stages < rhs.bindings[i].stages) return false;
		if (lhs.bindings[i].type > rhs.bindings[i].type) return true;
		else if (lhs.bindings[i].type < rhs.bindings[i].type) return false;
	}
	return false; // equal
}
bool operator==(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	if (lhs.count != rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.bindings[i].count != rhs.bindings[i].count) return false;
		if (lhs.bindings[i].stages != rhs.bindings[i].stages) return false;
		if (lhs.bindings[i].type != rhs.bindings[i].type) return false;
	}
	return true; // equal
}
bool operator!=(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	if (lhs.count != rhs.count) return true;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.bindings[i].count != rhs.bindings[i].count) return true;
		if (lhs.bindings[i].stages != rhs.bindings[i].stages) return true;
		if (lhs.bindings[i].type != rhs.bindings[i].type) return true;
	}
	return false; // equal
}

bool Program::hasVertexStage() const
{
	return vertex.data != nullptr;
}
bool Program::hasFragmentStage() const
{
	return fragment.data != nullptr;
}
bool Program::hasGeometryStage() const
{
	return geometry.data != nullptr;
}
bool Program::hasComputeStage() const
{
	return compute.data != nullptr;
}

ProgramHandle Program::createVertex(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* bindings, uint32_t count)
{
	return Application::app()->graphic()->createGraphicProgram(name, vertex, fragment, ShaderHandle::null, bindings, count);
}
ProgramHandle Program::createGeometry(const char* name, ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* bindings, uint32_t count)
{
	return Application::app()->graphic()->createGraphicProgram(name, vertex, fragment, geometry, bindings, count);
}
ProgramHandle Program::createCompute(const char* name, ShaderHandle compute, const ShaderBindingState* bindings, uint32_t count)
{
	return Application::app()->graphic()->createComputeProgram(name, compute, bindings, count);
}
void Program::destroy(ProgramHandle program)
{
	Application::app()->graphic()->destroy(program);
}

};
};