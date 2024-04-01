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

// Signed integer
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

// Floating point
using f16 = ::geometry::half;
using f32 = float;
using f64 = double;

using usize = ::std::size_t;

constexpr s32 assint(f32 value) {
	static_assert(sizeof(s32) == sizeof(f32));
	return reinterpret_cast<s32&>(value);
}
constexpr s64 assint(f64 value) {
	static_assert(sizeof(s64) == sizeof(f64));
	return reinterpret_cast<s64&>(value);
}
constexpr u32 asuint(f32 value) {
	static_assert(sizeof(u32) == sizeof(f32));
	return reinterpret_cast<u32&>(value);
}
constexpr u64 asuint(f64 value) {
	static_assert(sizeof(u64) == sizeof(f64));
	return reinterpret_cast<u64&>(value);
}
constexpr f32 asfloat(u32 value) {
	static_assert(sizeof(u32) == sizeof(f32));
	return reinterpret_cast<f32&>(value);
}
constexpr f64 asfloat(u64 value) {
	static_assert(sizeof(f64) == sizeof(u64));
	return reinterpret_cast<f64&>(value);
}
constexpr f32 asfloat(s32 value) {
	static_assert(sizeof(f32) == sizeof(s32));
	return reinterpret_cast<f32&>(value);
}
constexpr f64 asfloat(s64 value) {
	static_assert(sizeof(f64) == sizeof(s64));
	return reinterpret_cast<f64&>(value);
}

using float2 = ::geometry::vec2<f32>;
using float3 = ::geometry::vec3<f32>;
using float4 = ::geometry::vec4<f32>;

namespace simd {
// https://github.com/redorav/hlslpp
};

};