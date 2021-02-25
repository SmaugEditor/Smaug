#pragma once
#include <glm/vec3.hpp>
#include "mesh.h"

// Ray and line have the same structure, but have different purposes

struct ray_t
{
	glm::vec3 origin;
	
	// Direction
	glm::vec3 dir;
};

struct line_t
{
	glm::vec3 origin;

	// Magnitude and direction
	glm::vec3 delta;
};


// If hit is false, no data after it is guaranteed to be valid!
struct test_t
{
	bool hit = false;
};

struct testRayPlane_t : public test_t
{
	glm::vec3 intersect;
	
	// Normal of the plane
	glm::vec3 normal;

	// Dot of the line and tri
	float approach;

	// Parametric point of intersection
	float t = FLT_MAX;

};


struct testLineLine_t : public test_t
{
	glm::vec3 intersect;

};

// Test if a ray hits any geo in the world
testRayPlane_t testRay(ray_t ray);
// Test if a line hits any geo in the world before running out
testRayPlane_t testLine(line_t line);

bool testPointInAABB(glm::vec3 point, aabb_t aabb);
// sizes up the aabb by aabbBloat units before testing
bool testPointInAABB(glm::vec3 point, aabb_t aabb, float aabbBloat);

testLineLine_t testLineLine(line_t a, line_t b, float tolerance = 0.01f);

bool testPointInTri(glm::vec3 p, glm::vec3 tri0, glm::vec3 tri1, glm::vec3 tri2);