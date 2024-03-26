#include <Aka/Graphic/Program.h>

#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

Program::Program(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount) :
	Resource(name, ResourceType::Program),
	task(ShaderHandle::null),
	mesh(ShaderHandle::null),
	vertex(vertex),
	fragment(fragment),
	compute(ShaderHandle::null),
	sets{},
	constants{},
	setCount(bindingCounts),
	constantCount(constantCount)
{
	AKA_ASSERT(bindingCounts < ShaderMaxBindingCount, "Too much bindings");
	AKA_ASSERT(constantCount < ShaderMaxConstantCount, "Too much constants");
	memcpy(this->sets, sets, bindingCounts * sizeof(ShaderBindingState));
	memcpy(this->constants, constants, constantCount * sizeof(ShaderConstant));
}

Program::Program(const char* name, ShaderHandle task, ShaderHandle mesh, ShaderHandle fragment, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount) :
	Resource(name, ResourceType::Program),
	task(task),
	mesh(mesh),
	vertex(ShaderHandle::null),
	fragment(fragment),
	compute(ShaderHandle::null),
	sets{},
	constants{},
	setCount(bindingCounts),
	constantCount(constantCount)
{
	AKA_ASSERT(bindingCounts < ShaderMaxBindingCount, "Too much bindings");
	AKA_ASSERT(constantCount < ShaderMaxConstantCount, "Too much constants");
	memcpy(this->sets, sets, bindingCounts * sizeof(ShaderBindingState));
	memcpy(this->constants, constants, constantCount * sizeof(ShaderConstant));
}
Program::Program(const char* name, ShaderHandle compute, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount) :
	Resource(name, ResourceType::Program),
	task(ShaderHandle::null),
	mesh(ShaderHandle::null),
	vertex(ShaderHandle::null),
	fragment(ShaderHandle::null),
	compute(compute),
	sets{},
	constants{},
	setCount(bindingCounts),
	constantCount(constantCount)
{
	AKA_ASSERT(bindingCounts < ShaderMaxBindingCount, "Too much bindings");
	AKA_ASSERT(constantCount < ShaderMaxConstantCount, "Too much constants");
	memcpy(this->sets, sets, bindingCounts * sizeof(ShaderBindingState));
	memcpy(this->constants, constants, constantCount * sizeof(ShaderConstant));
}

bool operator<(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	if (lhs.count < rhs.count) return true;
	else if (lhs.count > rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.bindings[i].flags < rhs.bindings[i].flags) return true;
		else if (lhs.bindings[i].flags > rhs.bindings[i].flags) return false;
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
		if (lhs.bindings[i].flags > rhs.bindings[i].flags) return true;
		else if (lhs.bindings[i].flags < rhs.bindings[i].flags) return false;
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
		if (lhs.bindings[i].flags != rhs.bindings[i].flags) return false;
		if (lhs.bindings[i].count != rhs.bindings[i].count) return false;
		if (lhs.bindings[i].stages != rhs.bindings[i].stages) return false;
		if (lhs.bindings[i].type != rhs.bindings[i].type) return false;
	}
	return true; // equal
}
bool operator!=(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	return !operator==(lhs, rhs);
}

bool Program::hasVertexStage() const
{
	return vertex != ShaderHandle::null;
}
bool Program::hasFragmentStage() const
{
	return fragment != ShaderHandle::null;
}
bool Program::hasMeshStage() const
{
	return mesh != ShaderHandle::null;
}
bool Program::hasTaskStage() const
{
	return task != ShaderHandle::null;
}
bool Program::hasComputeStage() const
{
	return compute != ShaderHandle::null;
}

ProgramHandle Program::createVertex(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* bindings, uint32_t count, const ShaderConstant* constants, uint32_t constantCount)
{
	return Application::app()->graphic()->createVertexProgram(name, vertex, fragment, bindings, count, constants, constantCount);
}
ProgramHandle Program::createMesh(const char* name, ShaderHandle task, ShaderHandle mesh, ShaderHandle fragment, const ShaderBindingState* bindings, uint32_t count, const ShaderConstant* constants, uint32_t constantCount)
{
	return Application::app()->graphic()->createMeshProgram(name, task, mesh, fragment, bindings, count, constants, constantCount);
}
ProgramHandle Program::createCompute(const char* name, ShaderHandle compute, const ShaderBindingState* bindings, uint32_t count, const ShaderConstant* constants, uint32_t constantCount)
{
	return Application::app()->graphic()->createComputeProgram(name, compute, bindings, count, constants, constantCount);
}
void Program::destroy(ProgramHandle program)
{
	Application::app()->graphic()->destroy(program);
}

};
};