#pragma once

#include <stdint.h>

namespace aka {

namespace gpu {
// Include some glsl here

// float
// double
using uint = uint32_t;
// int
struct alignas(16) uint2 {};
struct alignas(16) uint3 {};
struct alignas(16) uint4 {};
struct alignas(16) int2 {};
struct alignas(16) int3 {};
struct alignas(16) int4 {};
struct alignas(16) float2 {};
struct alignas(16) float3 {};
struct alignas(16) float4 {};
struct alignas(16) double2 {};
struct alignas(16) double3 {};
struct alignas(16) double4 {};
struct alignas(16) float4x4 { float4 cols[4]; };
struct alignas(16) float3x3 { float3 cols[3]; };
//struct alignas(16) float3x4 { float4 cols[3]; };

uint InvalidIndex = (uint)-1;
}


};