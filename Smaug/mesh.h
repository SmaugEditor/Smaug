#pragma once
#include "glm/vec3.hpp"
#include <vector>

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
struct mesh_t;

struct vertex_t
{
	// Should refer back to the face's verts
	glm::vec3* vert;

	// The edge that stems out of this vert
	halfEdge_t* edge = nullptr;
};


struct halfEdge_t
{
	// This edge stems out of a vertex. vert is the vertex at the end of this edge.
	vertex_t* vert = nullptr;

	// This edge is half of another edge. This is the other side of the edge located on another face.
	halfEdge_t* pair = nullptr;

	// The face this edge belongs to.
	face_t* face = nullptr;

	// Next edge. Same as vert->edge.
	halfEdge_t* next = nullptr;
};


struct meshPart_t;
struct face_t
{
	size_t sideCount;

	// These edges use the verts 
	std::vector<halfEdge_t*> edges;

	std::vector<vertex_t*> verts;

	// What part of the mesh we belong to.
	meshPart_t* meshPart;
};

// Axis Aligned Bounding Box. Absolute minimum and maximum of a shape
struct aabb_t
{
	glm::vec3 min;
	glm::vec3 max;
};

struct meshPart_t
{
	std::vector<face_t*> faces;
	
	// Calculated from face verts
	//aabb_t aabb;
};

struct mesh_t
{
	std::vector<meshPart_t*> parts;

	// Calculated from part aabbs
	aabb_t aabb;
};



// One shape contains many HE'd faces
// Used for large editing and faces for cutting
//
//  Shape
//
//      /\
//     /  \
//    /    \
//   /      \
//  /        \
// /__________\
//
//  Faces of the shape after cut
//
//      /\
//     /  \
//    /____\
//   / |XX| \
//  /  |XX|  \
// /___|__|___\

struct cutttableShape_t : public meshPart_t
{
	// These define the shape 
	// Must be defined in clockwise-faceout-order
	// Editing this MUST be followed by redefinition!
	std::vector<glm::vec3> verts;

	// Inherited from meshPart_t
	// These are triangluated faces within the shape
	// Never depend on them post a refresh
	//std::vector<face_t*> faces;

	// These cut into this shape and form what the faces look like
	std::vector<cutttableShape_t*> cutters;
};