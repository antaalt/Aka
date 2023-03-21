#pragma once

#include <Aka/Core/StrictType.h>
#include <Aka/Core/Geometry.h>

#include <stdint.h>

namespace aka {

// Unsigned integer
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
//using uint128 = __int128;

// Signed integer
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// Floating point
using f16 = geometry::half;
using f32 = float;
using f64 = double;
//using f128 = long double;


using float2 = geometry::vec2<f32>;
using float3 = geometry::vec3<f32>;
using float4 = geometry::vec4<f32>;

namespace simd {



};

};