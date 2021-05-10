#pragma once

#include "sassert.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <bgfx/bgfx.h>
#include <cmath>

const double PI = 3.141592653589793238463;

// This function does not take Y into account
bool IsPointOnLine2D(glm::vec3 point1, glm::vec3 point2, glm::vec3 mouse, float range);

bool IsPointNearPoint2D(glm::vec3 point1, glm::vec3 point2, float range);

enum class SolveToLine2DSnap
{
	POINT,
	Y_SNAP, // Y was solved to the line, X is free
	X_SNAP, // X was solved to the line, Y is free
};
glm::vec2 SolveToLine2D(glm::vec2 pos, glm::vec2 lineStart, glm::vec2 lineEnd, SolveToLine2DSnap* snap = nullptr);

bgfx::ProgramHandle LoadShader(const char* fragment, const char* vertex);

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

template<bool unordered1 = false, bool unordered2 = false>
inline constexpr bool rangeInRange(float min1, float max1, float min2, float max2)
{
	return inRange<unordered1>(min1, max1, min2) || inRange<unordered2>(min1, max1, max2);
}

inline constexpr bool closeTo(float value, float target, float threshold = 0.0001f)
{
	return inRange(target - threshold, target + threshold, value);
}


inline constexpr glm::vec3 dirMask(glm::vec3 vec)
{
	return { 1.0f - std::fabs(vec.x), 1.0f - std::fabs(vec.y), 1.0f - std::fabs(vec.z) };
}


#define COLOR_RED   glm::vec3( 1.0, 0.0, 0.0 )
#define COLOR_GREEN glm::vec3( 0.0, 1.0, 0.0 )
#define COLOR_BLUE  glm::vec3( 0.0, 0.0, 1.0 )
#define COLOR_BLACK glm::vec3( 0.0, 0.0, 0.0 )
#define COLOR_WHITE glm::vec3( 1.0, 1.0, 1.0 )
#define COLOR_GRAY  glm::vec3( 0.5, 0.5, 0.5 )

inline constexpr glm::vec3 colorHSV(float hue, float saturation, float value)
{
	const float hueRange = 2 * PI;
	const float onesixth = hueRange / 6;
	float oR = clamp<float>((std::fabs(hue - hueRange / 2.0f) - onesixth) / onesixth, 0.0f, 1.0f);
	oR = (oR * saturation + (1.0f - saturation)) * value;
	float oG = clamp<float>((onesixth * 2.0f - std::fabs(hue - onesixth * 2)) / onesixth, 0.0f, 1.0f);
	oG = (oG * saturation + (1.0f - saturation)) * value;
	float oB = clamp<float>((onesixth * 2.0f - std::fabs(hue - onesixth * 4)) / onesixth, 0.0f, 1.0f);
	oB = (oB * saturation + (1.0f - saturation)) * value;
	return glm::vec3(oR, oG, oB);
}
inline constexpr glm::vec3 colorHSV(glm::vec3 vec) { return colorHSV(vec.x, vec.y, vec.z); }

inline glm::vec3 randColorHue()
{
	return colorHSV(rand() % 1000 / 1000.0f * PI * 2, 1, 1);
}


#define MAKE_BITFLAG(enumName) \
inline enumName& operator |=  (enumName& a, enumName b) { a = static_cast<enumName>(a | b);  return a; } \
inline enumName& operator &=  (enumName& a, enumName b) { a = static_cast<enumName>(a & b);  return a; } \
inline enumName& operator ^=  (enumName& a, enumName b) { a = static_cast<enumName>(a ^ b);  return a; } \
inline enumName& operator <<= (enumName& a, int b)      { a = static_cast<enumName>(a >> b); return a; } \
inline enumName& operator >>= (enumName& a, int b)      { a = static_cast<enumName>(a << b); return a; } \
inline enumName  operator ~   (enumName a) { return static_cast<enumName>(~(char)a); }
