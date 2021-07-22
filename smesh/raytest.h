#pragma once
#include <glm/vec3.hpp>
#include <cfloat>
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

	float t1 = FLT_MAX;
	float t2 = FLT_MAX;
};

struct tri_t
{
	glm::vec3 verts[3];

	glm::vec3& a() { return verts[0]; };
	glm::vec3& b() { return verts[1]; };
	glm::vec3& c() { return verts[2]; };

	glm::vec3& operator[](int i) { return verts[i]; }
};


/////////////////////
// Local Geo Tests //
/////////////////////

bool testPointInAABB(glm::vec3 point, aabb_t aabb);
// sizes up the aabb by aabbBloat units before testing
bool testPointInAABB(glm::vec3 point, aabb_t aabb, float aabbBloat);
bool testAABBInAABB(aabb_t a, aabb_t b, float aabbBloat = 0.0f);

testLineLine_t testLineLine(line_t a, line_t b, float tolerance = 0.01f);
inline testLineLine_t testLineLine(halfEdge_t* a, halfEdge_t* b, glm::vec3 aOrigin, glm::vec3 bOrigin, float tolerance = 0.01f)
{
	glm::vec3 aStem = *a->vert->vert;
	glm::vec3 bStem = *b->vert->vert;

	return testLineLine({aStem + aOrigin, *a->next->vert->vert - aStem}, { bStem + bOrigin, *b->next->vert->vert - bStem }, tolerance);
}


testRayPlane_t pointOnPartLocal(meshPart_t* part, glm::vec3 p);

// Wish this could be a template...
bool testPointInTriNoEdges(glm::vec3 p, glm::vec3 tri0, glm::vec3 tri1, glm::vec3 tri2);
bool testPointInTriEdges(glm::vec3 p, glm::vec3 tri0, glm::vec3 tri1, glm::vec3 tri2);

// Templates pleaase
testRayPlane_t rayVertLoopTest(ray_t ray, vertex_t* vert, float closestT);
testRayPlane_t rayVertLoopTestNoCull(ray_t ray, vertex_t* vert, float closestT);

void rayAABBTest(ray_t ray, aabb_t aabb, testRayPlane_t& lastTest);