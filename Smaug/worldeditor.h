#pragma once
#include <glm/vec3.hpp>
#include <vector>


enum class Constraint
{
	NONE,
	LOCKED_TO_PARENT,

};

struct nodeSide_t
{
	glm::vec3 point1;
	glm::vec3* point2; // Links to the next side's point 1
	
	Constraint constraint;
	std::vector<nodeSide_t> linkedSides;
	nodeSide_t* parentSide;
};

struct node_t
{
	
	std::vector<nodeSide_t> sides;

};

class CWorldEditor
{

};

