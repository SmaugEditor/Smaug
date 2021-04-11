#include "meshtest.h"
#include <glm/geometric.hpp>

bool pointInConvexMeshPart(meshPart_t* part, glm::vec3 pos)
{
	vertex_t* vs = part->verts.front(), *v = vs, *v1 = vs->edge->vert, *v2 = v1->edge->vert;
	
	// Get the normal
	glm::vec3 edge1 = (*vs->vert) - (*v1->vert);
	glm::vec3 edge2 = (*v1->vert) - (*v2->vert);
	glm::vec3 norm = glm::cross(edge1, edge2);

	do
	{	
		glm::vec3 delta = *v->edge->vert->vert - *v->vert;
		glm::vec3 perp = glm::cross(delta, norm);
		float m = glm::dot(pos - *v->vert, perp);

		// If pos has an m > 0, it's in front of the edge, out of bounds
		if (m > 0)
			return false;

		// Please don't pass me a weird mesh...
		v = v->edge->vert;
	} while (v != vs);

	// We're 100% inside this convex part
	return true;
}