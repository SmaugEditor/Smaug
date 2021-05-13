#include "utils.h"
#include <bigg.hpp>
#include <cstring>
#include <charconv>
#include <cstdio>

// Keep this private
RendererProperties_t gRenderProps;

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

bgfx::ProgramHandle LoadShader(const char* fragment, const char* vertex)
{
	// Static so we don't reinitiate this
	static const char** shaderPaths = new const char* [10]
	{
		nullptr,		  // No Renderer
		"shaders/dx9/",   // Direct3D9
		"shaders/dx11/",  // Direct3D11
		"shaders/dx11/",  // Direct3D12
		nullptr,		  // Gnm
		nullptr,		  // Metal
		nullptr,		  // Nvm
		"shaders/essl/",  // OpenGL ES
		"shaders/glsl/",  // OpenGL
		"shaders/spirv/", // Vulkan
	};

	int type = bgfx::getRendererType();

	// Out of bounds. Return an invalid handle
	if (type >= bgfx::RendererType::Count || type < 0)
	{
		return BGFX_INVALID_HANDLE;
	}

	const char* shaderPath = shaderPaths[bgfx::getRendererType()];

	// Unsupported platform. Return invalid handle
	if (!shaderPath)
	{
		return BGFX_INVALID_HANDLE;
	}

	char vsPath[128];
	strcpy(vsPath, shaderPath);
	strcat(vsPath, vertex);

	char fsPath[128];
	strcpy(fsPath, shaderPath);
	strcat(fsPath, fragment);
	
	printf("LoadShader: Loading %s and %s\n", vsPath, fsPath);

	return bigg::loadProgram(vsPath, fsPath);
}

static const float s_MathPI = acos(-1);

void Directions(glm::vec3 angles, glm::vec3* forward, glm::vec3* right, glm::vec3* up)
{
	const float pitch = angles.x;
	const float yaw = angles.y;
	const float roll = -angles.z;

	glm::vec3 _forward;
	_forward.x = sin(yaw) * cos(pitch);
	_forward.y = -sin(pitch);
	_forward.z = cos(yaw) * cos(pitch);
	_forward = glm::normalize(_forward);

	if (forward)
		*forward = _forward;


	glm::vec3 _right;
	_right.x = -cos(roll) * cos(yaw);
	_right.y = sin(roll);
	_right.z = cos(roll) * sin(yaw);
	_right = glm::normalize(_right);


	if (right)
		*right = _right;

	if (up)
		*up = glm::normalize(glm::cross(_right, _forward));
/*
	if (up)
	{
		glm::vec3 _up;
		_up.x = cos(-(roll + yaw)) * sin(pitch);
		_up.y = cos(-roll) * cos(pitch);
		_up.z = sin(-roll) * cos(yaw) * ((sin(pitch) + 1)/2);
		_up = glm::normalize(_up);

		*up = _up;
	}
	*/
}

glm::vec3 Angles(glm::vec3 forward, glm::vec3* up)
{
	glm::vec3 right = glm::cross(forward, up ? *up : glm::vec3(0, 1, 0));

	forward = glm::normalize(forward);
	float pitch = asin(-forward.y);
	float roll = asin(right.y);

	float cosPitch = cos(pitch);
	float cosRoll = cos(roll);
	float yaw = cosPitch != 0.0f ? acos(forward.z / cos(pitch)) : ( cosRoll != 0.0f ? acos(-right.x / cosRoll ) : 0.0f);

	if (forward.x < 0) yaw *= -1;



	return { pitch, yaw, roll };
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

void SetRendererType(bgfx::RendererType::Enum type)
{
	auto old = gRenderProps.renderType;
	gRenderProps.renderType = type;
	
	// Init the rest of the fields 
	if(old != type)
	{
		using RT = bgfx::RendererType::Enum;
		switch(type)
		{
			case RT::Direct3D11:
			case RT::Direct3D12:
			case RT::Direct3D9:
				gRenderProps.coordSystem = ECoordSystem::LEFT_HANDED; 
				break;
			default:
				gRenderProps.coordSystem = ECoordSystem::RIGHT_HANDED;
		}
	}
	
}

bgfx::RendererType::Enum RendererType()
{
	return gRenderProps.renderType;
}

const RendererProperties_t& RendererProperties()
{
	return gRenderProps;
}