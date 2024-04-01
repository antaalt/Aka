#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in vec4 a_color;

#ifdef SKELETAL
const uint MaxBoneInfluence = 4;// TODO shared code
const uint InvalidBoneID = uint(-1);
// Vertex
layout (location = 4) in uvec4 a_boneIndex;
layout (location = 5) in vec4 a_weights;
// Instance
layout(location = 6) in mat4 a_worldMatrix;
layout(location = 10) in mat4 a_normalMatrix;
layout(location = 14) in uint a_batchIndex;
layout(location = 15) in uint a_boneOffset;
#else // STATIC
// Instance
layout(location = 4) in mat4 a_worldMatrix;
layout(location = 8) in mat4 a_normalMatrix;
layout(location = 12) in uint a_batchIndex;
#endif

// Output
layout (location = 0) out vec4 v_worldPosition;
layout (location = 1) out vec3 v_worldNormal;
layout (location = 2) out vec2 v_uv;
layout (location = 3) out vec4 v_color;
layout (location = 4) out flat uint v_materialID;

layout(set = 0, binding = 0) uniform CameraUniformBuffer {
	mat4 view;
	mat4 projection;
} u_camera;

struct MaterialData
{
	vec4 color;
	uint albedoID;
	uint normalID;
};

layout(std140, set = 1, binding = 0) readonly buffer MaterialDataBuffer {
	MaterialData data[];
} u_material;

// TODO share these struct between C++ & glsl
struct AssetData
{
	uint batchOffset;
	uint batchCount; // valid batches
};

struct BatchData
{
	uint vertexOffset;
	uint indexOffset;
	uint indexCount;

	uint materialIndex;
	// BBOX
	vec4 min;
	vec4 max;
};


layout(std140, set = 3, binding = 0) readonly buffer AssetDataBuffer {
	AssetData data[];
} u_assets;

layout(std140, set = 3, binding = 1) readonly buffer BatchDataBuffer {
	BatchData data[];
} u_batches;

#ifdef SKELETAL
// TODO raw buffer instead
layout(std140, set = 3, binding = 2) readonly buffer BoneDataBuffer {
	mat4 offset[];
} u_bones;
#endif

void main(void)
{
	vec3 localPosition = a_position;
	vec3 localNormal = a_normal;
#ifdef SKELETAL // Rigging
	vec3 sumPosition = vec3(0.0);
	vec3 sumNormal = vec3(0.0);
	for(int i = 0 ; i < MaxBoneInfluence; i++)
	{
		if(a_boneIndex[i] == InvalidBoneID) 
			continue;
		vec4 boneLocalPosition = u_bones.offset[a_boneOffset + a_boneIndex[i]] * vec4(localPosition,1.0);
		vec3 boneLocalNormal = mat3(u_bones.offset[a_boneOffset + a_boneIndex[i]]) * localNormal;
		sumPosition += boneLocalPosition.xyz * a_weights[i];
		sumNormal   += boneLocalNormal * a_weights[i];
	}
	localPosition = sumPosition;
	localNormal = sumNormal;
#endif
	v_worldPosition = a_worldMatrix * vec4(localPosition, 1.0);
	v_worldNormal = normalize(mat3(a_normalMatrix) * localNormal);
	v_uv = a_uv;
	v_color = a_color;
	v_materialID = u_batches.data[a_batchIndex].materialIndex;
	gl_Position = u_camera.projection * u_camera.view * v_worldPosition;
}
