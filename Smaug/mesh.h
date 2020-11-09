#pragma once
#include "glm/vec3.hpp"


/*

Half Edge data structure
http://sccg.sk/~samuelcik/dgs/half_edge.pdf

V=vertex
E=vertex.edge.vertex
N=vertex.edge.next.vertex
P=vertex.edge.next.next.pair.next.vertex

     E
    / \
   V---N
    \ /
     P
	 

*/


struct halfEdge_t;
struct face_t;

// Inherit from glm::vec3 for easy compatibility
struct vertex_t : public glm::vec3 
{
	// The edge that stems out of this vert
	halfEdge_t* edge;
};


struct halfEdge_t
{
	// This edge stems out of a vert. vert is the vert at the end of it.
	vertex_t* vert;

	// This edge is half of another edge. This is the other side of the edge located on another face.
	halfEdge_t* pair;

	// The face this edge belongs to.
	face_t* face;

	// Next edge. Same as vert->edge.
	halfEdge_t* next;
};


// Each face is a triangle
struct face_t
{
	// These edges use the verts 
	halfEdge_t edges[3];

	vertex_t verts[3];
};

struct mesh_t
{

};