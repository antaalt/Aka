#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "asset.glsl"

layout(location = 0) in vec4 v_worldPosition;
layout(location = 1) in vec3 v_worldNormal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec4 v_color;
layout(location = 4) in flat uint v_materialID;

layout(location = 0) out vec4 o_color;

//https://vkguide.dev/docs/chapter-4/storage_buffers/
layout(std140, set = 1, binding = 0) readonly buffer MaterialDataBuffer {
	MaterialData data[];
} u_material;

layout(set = 2, binding = 0) uniform sampler2D  u_textures[];

/*layout( push_constant ) uniform constants {
	uint materialID; // TODO link to instanceData
} u_push;*/

void main()
{
	vec3 sunDir = vec3(0, 1, 0);
	float cosTheta = dot(sunDir, normalize(v_worldNormal));
	MaterialData material = u_material.data[v_materialID]; // cant use v_materialID yet, cuz its per instance & not batch.
	vec4 albedo = texture(u_textures[material.albedoID], v_uv) * material.color * v_color;
	vec4 ambientLight = albedo * 0.2;
	vec4 sunLight = albedo * cosTheta;
	o_color = ambientLight + sunLight;
}
