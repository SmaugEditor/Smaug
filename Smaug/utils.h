#pragma once

#include "log.h"
#include "shared.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <cmath>
#include <initializer_list>


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



#define COLOR_RED   glm::vec3( 1.0, 0.0, 0.0 )
#define COLOR_GREEN glm::vec3( 0.0, 1.0, 0.0 )
#define COLOR_BLUE  glm::vec3( 0.0, 0.0, 1.0 )
#define COLOR_BLACK glm::vec3( 0.0, 0.0, 0.0 )
#define COLOR_WHITE glm::vec3( 1.0, 1.0, 1.0 )
#define COLOR_GRAY  glm::vec3( 0.5, 0.5, 0.5 )

inline glm::vec3 colorHSV(float hue, float saturation, float value)
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

inline glm::vec3 colorHSV(glm::vec3 vec) { return colorHSV(vec.x, vec.y, vec.z); }

inline glm::vec3 randColorHue()
{
	return colorHSV(rand() % 1000 / 1000.0f * PI * 2, 1, 1);
}



namespace CommandLine
{
	void Set(int argc, const char* const* argv);
	
	bool HasParam(const char* param);
	const char* GetParam(const char* param);
	int GetInt(const char* param);
	float GetFloat(const char* param);
	
	template<class ...T>
	inline bool HasAny(T...params)
	{
		const char* args[] = {
			params...
		};
		for(int i = 0; i < sizeof...(params); i++)
			if(HasParam(args[i]))
				return true;
		return false;
	}
}
