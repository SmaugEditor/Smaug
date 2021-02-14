#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <bgfx/bgfx.h>

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


struct key_t
{
public:
	
	int key;
};

struct keyName_t
{
	int id;
	const char* name;
};

keyName_t* GetAllKeys(size_t* length);
key_t KeyFromName(const char* name);
const char* KeyToName(key_t key);


template<typename T>
T max(T a, T b)
{
	if (a > b)
		return a;
	return b;
}

template<typename T>
T min(T a, T b)
{
	if (a < b)
		return a;
	return b;
}


template<typename T>
T clamp(T min, T max, T value)
{
	if (value > max)
		return max;
	if (value < min)
		return min;
	return value;
}

