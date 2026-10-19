#pragma once
#include <cstdint>

using int64 = int64_t;
using uint64 = uint64_t;
using uint = uint32_t;
using ushort = uint16_t;
using byte = uint8_t;

template <typename T>
struct vec2_base
{
	union
	{
		T vals[2];
		struct
		{
			T x;
			T y;
		};
	};

	T &operator[]( uint i ) { return vals[i]; }
};

template <typename T>
bool operator==( const vec2_base<T> &a, const vec2_base<T> &b ) { return a.x == b.x && a.y == b.y; }

using vec2 = vec2_base<float>;
using ivec2 = vec2_base<int>;
using uvec2 = vec2_base<uint>;


template <typename T>
struct vec3_base
{
	union
	{
		T vals[3];
		struct
		{
			T x;
			T y;
			T z;
		};
	};

	T &operator[]( uint i ) { return vals[i]; }
};

template <typename T>
bool operator==( const vec3_base<T> &a, const vec3_base<T> &b ) { return a.x == b.x && a.y == b.y && a.z == b.z; }

using vec3 = vec3_base<float>;
using ivec3 = vec3_base<int>;
using uvec3 = vec3_base<uint>;