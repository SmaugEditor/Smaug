#pragma once
#include <glm/vec3.hpp>
#include "mesh.h"

struct line_t
{
	glm::vec3 origin;
	glm::vec3 delta;
};

// If hit is false, no data after it is guaranteed to be valid!
struct test_t
{
	bool hit = false;
};

struct testLine_t : public test_t
{
	glm::vec3 hitPos;
	
	// Normal of the plane
	glm::vec3 normal;

	// Dot of the line and tri
	float approach;

	// Parametric point of intersection
	float t = FLT_MAX;

};

testLine_t testLine(line_t line);
