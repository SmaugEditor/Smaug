#pragma once
#include "raytest.h"


/////////////////////
// World Geo Tests //
/////////////////////

class CNode;
struct testWorld_t : public testRayPlane_t
{
	CNode* node;
	meshPart_t* part;
	face_t* face;
};

// Test if a ray hits any geo in the world
testWorld_t testRay(ray_t ray);
// Test if a line hits any geo in the world before running out
testWorld_t testLine(line_t line);
