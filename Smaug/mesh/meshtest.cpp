#include "meshtest.h"
#include <glm/geometric.hpp>

bool pointInConvexMeshPart(meshPart_t* part, glm::vec3 pos)
{
	for (auto f : part->collision)
		if(pointInConvexMeshFace(f, pos))
			return true;
	return false;
}

template<bool testEdges>
bool pointInConvexLoop(vertex_t* vert, glm::vec3 pos)
{
	vertex_t* vs = vert, * v = vs, * v1 = vs->edge->vert, * v2 = v1->edge->vert;

	// Get the normal
	glm::vec3 edge1 = (*vs->vert) - (*v1->vert);
	glm::vec3 edge2 = (*v1->vert) - (*v2->vert);
	glm::vec3 norm = glm::cross(edge1, edge2);

	do
	{	
		vertex_t* next = v->edge->vert;

		glm::vec3 delta = *next->vert - *v->vert;
		glm::vec3 perp = glm::cross(delta, norm);
		float m = glm::dot(pos - *v->vert, perp);

		// If pos has an m > 0, it's in front of the edge, out of bounds
		if constexpr (testEdges)
		{
			if (m > 0)
				return false;
		}
		else
		{
			if (m >= 0)
				return false;
		}

		// Please don't pass me a weird mesh...
		v = next;
	} while (v != vs);

	// We're 100% inside this convex part
	return true;
}

bool pointInConvexLoop(vertex_t* vert, glm::vec3 pos) { return pointInConvexLoop<true>(vert, pos); }
bool pointInConvexLoopNoEdges(vertex_t* vert, glm::vec3 pos) { return pointInConvexLoop<false>(vert, pos); }
