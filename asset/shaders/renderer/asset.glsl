#ifndef ASSET_GLSL
#define ASSET_GLSL

struct MaterialData
{
	vec4 color;
	uint albedoID;
	uint normalID;
};

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

#endif