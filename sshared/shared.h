#pragma once
#include <glm/vec3.hpp>

// Dummy stuff so when we rewire this to be bgfx independent, it's easier
typedef void* texture_t;
#define INVALID_TEXTURE 0

const double PI = 3.141592653589793238463;

#define ZERO_VEC3 {0.0f, 0.0f, 0.0f}


// Pitch, Yaw, Roll
void Directions(glm::vec3 angles, glm::vec3* forward = nullptr, glm::vec3* right = nullptr, glm::vec3* up = nullptr);
glm::vec3 Angles(glm::vec3 forward, glm::vec3* up = nullptr);


template<typename T>
constexpr T max(T a, T b)
{
	if (a > b)
		return a;
	return b;
}

template<typename T>
constexpr T min(T a, T b)
{
	if (a < b)
		return a;
	return b;
}


template<typename T>
constexpr T clamp(T value, T min, T max)
{
	if (value > max)
		return max;
	if (value < min)
		return min;
	return value;
}

template<bool unordered = false>
inline constexpr bool inRange(float min, float max, float value)
{
	if constexpr (unordered)
	{
		if (min > max)
		{
			float t = min;
			min = max;
			max = t;
		}
	}
	return (value >= min) && (value <= max);
}

template<bool unordered = false>
inline constexpr bool rangeInRange(float largerMin, float largerMax, float smallerMin, float smallerMax)
{
	return inRange<unordered>(largerMin, largerMax, smallerMin) || inRange<unordered>(largerMin, largerMax, smallerMax);
}

template<bool unordered1 = false, bool unordered2 = false>
inline constexpr bool rangeOverlap(float min1, float max1, float min2, float max2)
{
	return rangeInRange<unordered1>(min1, max1, min2, max2) || rangeInRange<unordered2>(min2, max2, min1, max1);
}

// Certain operations introduce more error than others

// General purpose
#define EPSILON 0.0001f
// Use this while working with faces, doing dots and crosses
#define EPSILON_FACE 0.001f
// For comparing points
#define EPSILON_POINT 0.0001f

inline constexpr bool closeTo(float value, float target, float threshold = EPSILON)
{
	value -= target;
	return value <= threshold && value >= -threshold;
}

inline constexpr bool closeTo(glm::vec3 value, glm::vec3 target, float threshold = EPSILON_POINT)
{
	value -= target;
	return value.x <=  threshold && value.y <=  threshold && value.z <=  threshold 
		&& value.x >= -threshold && value.y >= -threshold && value.z >= -threshold;
}


inline constexpr glm::vec3 dirMask(glm::vec3 vec)
{
	return { 1.0f - fabs(vec.x), 1.0f - fabs(vec.y), 1.0f - fabs(vec.z) };
}


#define MAKE_BITFLAG(enumName) \
inline enumName& operator |=  (enumName& a, enumName b) { a = static_cast<enumName>(a | b);  return a; } \
inline enumName& operator &=  (enumName& a, enumName b) { a = static_cast<enumName>(a & b);  return a; } \
inline enumName& operator ^=  (enumName& a, enumName b) { a = static_cast<enumName>(a ^ b);  return a; } \
inline enumName& operator <<= (enumName& a, int b)      { a = static_cast<enumName>(a >> b); return a; } \
inline enumName& operator >>= (enumName& a, int b)      { a = static_cast<enumName>(a << b); return a; } \
inline enumName  operator ~   (enumName a) { return static_cast<enumName>(~(char)a); }