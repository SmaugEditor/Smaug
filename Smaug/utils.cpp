#include "utils.h"
#include <cstring>
#include <charconv>
#include <cstdio>
#include <glm/glm.hpp>


bool IsPointOnLine2D(glm::vec3 point1, glm::vec3 point2, glm::vec3 mouse, float range)
{
	// Top point is always point1
	if (point2.z > point1.z)
	{
		glm::vec3 temp = point1;
		point1 = point2;
		point2 = temp;
	}
	// If the Zs are equal, then the left point is always point1
	else if (point2.z == point1.z && point2.x < point1.x)
	{
		glm::vec3 temp = point1;
		point1 = point2;
		point2 = temp;
	}

	// Is the line is straight up?
	if (point1.x == point2.x)
	{
		return abs(mouse.x - point1.x) < range // Is within the width bounds?
			&& mouse.z <= point1.z  // Is not higher than
			&& mouse.z >= point2.z; // Is not lower than
	}

	// Is the line straight sideways?
	if (point1.z == point2.z)
	{
		return abs(mouse.z - point1.z) < range // Is within the height bounds?
			&& mouse.x >= point1.x  // Is not left of
			&& mouse.x <= point2.x; // Is not right of
	}

	float slope = (point1.z - point2.z) / (point1.x - point2.x);
	float perp = -1 / slope;

	float perpAngle = atan(perp);
	glm::vec3 perpOffset = glm::vec3(range * cos(perpAngle), 0, range * sin(perpAngle));

	glm::vec3 top = point1;
	glm::vec3 bottom = point2;

	
	if (slope < 0) // Line points down. The perpOffset is flipped
	{
		top -= perpOffset;
		bottom += perpOffset;
	}
	else // Line points up
	{
		top += perpOffset;
		bottom -= perpOffset;
	}

	// Bottom Slope
	if ((mouse.x - bottom.x) * slope + bottom.z < mouse.z)
		return false;

	// Top Slope
	if ((mouse.x - top.x) * slope + top.z > mouse.z)
		return false;

	// Top Perp
	if ((mouse.x - top.x) * perp + top.z < mouse.z)
		return false;

	// Bottom Perp
	if ((mouse.x - bottom.x) * perp + bottom.z > mouse.z)
		return false;

	return true;
}

bool IsPointNearPoint2D(glm::vec3 point1, glm::vec3 point2, float range)
{
	// Ignore Y
	point1.y = 0;
	point2.y = 0;

	return glm::length(point2 - point1) <= range;
}

glm::vec2 SolveToLine2D(glm::vec2 pos, glm::vec2 lineStart, glm::vec2 lineEnd, SolveToLine2DSnap* snap)
{
	glm::vec2 line = lineEnd - lineStart;
	if (line.x == 0 && line.y == 0)
	{
		// A Point. No line
		if (snap) *snap = SolveToLine2DSnap::POINT;
		return lineEnd;
	}
	else if (line.x == 0)
	{
		// Straight horizontal
		if (snap) *snap = SolveToLine2DSnap::X_SNAP;
		return { lineEnd.x, pos.y };
	}
	else if (line.y == 0)
	{
		// Straight vertical
		if (snap) *snap = SolveToLine2DSnap::Y_SNAP;
		return { pos.x, lineEnd.y };
	}

	float slope = line.y / line.x;

	if (fabs(slope) > 1)
	{
		// Vertical
		// Solve X onto the line
		slope = line.x / line.y;
		if (snap) *snap = SolveToLine2DSnap::X_SNAP;
		return glm::vec2((pos.y - lineStart.y) * slope + lineStart.x, pos.y);
	}
	else
	{
		// Horizontal
		// Solve Y onto the line
		if (snap) *snap = SolveToLine2DSnap::Y_SNAP;
		return glm::vec2(pos.x, (pos.x - lineStart.x) * slope + lineStart.y);
	}
}

namespace CommandLine
{
	const char* const* argv; 
	int argc;
}


void CommandLine::Set(int argc, const char* const* argv)
{
	CommandLine::argv = argv;
	CommandLine::argc = argc;
}
	
bool CommandLine::HasParam(const char* param)
{
	for(int i = 0; i < argc; i++)
		if(std::strcmp(param, argv[i]) == 0)
			return true;
	return false;
}

const char* CommandLine::GetParam(const char* param)
{
	for(int i = 0; i < argc; i++)
	{
		if(!std::strcmp(param, argv[i]))
		{
			if(i < argc-1)
				return argv[i+1];
			return nullptr;
		}
	}
	return nullptr;
}

int CommandLine::GetInt(const char* param)
{
	const char* val = GetParam(param);
	if(val)
		return std::atoi(val);
	return 0;
}

float CommandLine::GetFloat(const char* param)
{
	const char* val = GetParam(param);
	if(val)
		return std::atof(val);
	return 0.f;
}
