#include "meshtest.h"
#include <glm/geometric.hpp>

bool pointInConvexMeshPart(meshPart_t* part, glm::vec3 pos)
{
	for (auto f : part->collision)
		if(pointInConvexMeshFace(f, pos))
			return true;
	return false;
}

bool pointInConvexMeshPartNoEdges(meshPart_t* part, glm::vec3 pos)
{
	for (auto f : part->collision)
		if (pointInConvexLoopNoEdges(f->verts.front(), pos))
			return true;
	return false;
}
// We could theoretically half all sliced verts be on internal edges, so this function exists to mitigate that


template<bool testEdges>
bool pointInConvexLoop(vertex_t* vert, glm::vec3 pos)
{
	vertex_t* vs = vert, * v = vs, * v1 = vs->edge->vert, * v2 = v1->edge->vert;

	// Get the normal
	glm::vec3 norm = vertNextNormal(vert);

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

template<bool ignoreNonOuterEdges>
pointInConvexTest_t pointInConvexLoopQuery(vertex_t* vert, glm::vec3 pos)
{
	pointInConvexTest_t test;
	
	vertex_t* vs = vert, * v = vs, * v1 = vs->edge->vert, * v2 = v1->edge->vert;

	// Get the normal
	glm::vec3 norm = vertNextNormal(vert);

	do
	{

		vertex_t* next = v->edge->vert;

		glm::vec3 delta = *next->vert - *v->vert;
		glm::vec3 perp = glm::cross(delta, norm);
		float m = glm::dot(pos - *v->vert, perp);

		//if (m == 0)
		if(closeTo(m, 0, EPSILON_FACE))
		{
			if constexpr (ignoreNonOuterEdges)
			{
				if (!v->edge->pair)
					test.onEdge++;
				else
					test.inside++;
			}
			else
				test.onEdge++;
		}
		else if (m > 0)
			test.outside++;
		else if (m < 0)
			test.inside++;
		
		
		// Please don't pass me a weird mesh...
		v = next;
	} while (v != vs);

	return test;
}


pointInConvexTest_t pointInConvexLoopQuery(vertex_t* vert, glm::vec3 pos)
{
	return pointInConvexLoopQuery<false>(vert, pos);
}

pointInConvexTest_t pointInConvexLoopQueryIgnoreNonOuterEdges(vertex_t* vert, glm::vec3 pos)
{
	return pointInConvexLoopQuery<true>(vert, pos);
}

