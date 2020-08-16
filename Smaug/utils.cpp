#include "utils.h"
#include <bigg.hpp>

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
		"shaders/glsl/",  // OpenGL
		nullptr,		  // OpenGL ES
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

	char vsPath[32];
	strcpy(vsPath, shaderPath);
	strcat(vsPath, vertex);

	char fsPath[32];
	strcpy(fsPath, shaderPath);
	strcat(fsPath, fragment);

	return bigg::loadProgram(vsPath, fsPath);
}
