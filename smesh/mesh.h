#pragma once
#include "shared.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
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

// Might be worth turning this whole thing into a class of sorts?
// It's a little bit messy currently...
// We need some more memory efficiency in here too

// I wish enum class was better
enum FaceFlags : char
{
	FF_NONE = 0,
	FF_CUT = 1,
	FF_CONVEX = 2,
	FF_MESH_PART = 4
};
MAKE_BITFLAG(FaceFlags);
enum EdgeFlags : char
{
	EF_NONE = 0,
	EF_SLICED = 1, // An edge that has been sliced
	EF_PART_EDGE = 2, // An edge that belongs to the part
	EF_OUTER = 4, // An edge that's on the outer part of the face
};
MAKE_BITFLAG(EdgeFlags);

struct halfEdge_t;
struct face_t;
struct mesh_t;

struct vertex_t
{
	// Should refer back to the face's verts
	glm::vec3* vert = nullptr;

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

	EdgeFlags flags = EdgeFlags::EF_NONE;
};


struct meshPart_t;

struct face_t
{
	~face_t();

	// Do we really need vectors for this?

	// These edges use the verts 
	// Not ordered!
	std::vector<halfEdge_t*> edges;

	// Not ordered!
	std::vector<vertex_t*> verts;

	// What do we belong to? Could be a meshpart, face, or etc.
	face_t* parent = nullptr;


	FaceFlags flags = FaceFlags::FF_NONE;
};


// Axis Aligned Bounding Box. Absolute minimum and maximum of a shape
struct aabb_t
{
	glm::vec3 min;
	glm::vec3 max;
};

struct slicedMeshPartData_t
{
	~slicedMeshPartData_t();

	// Convexified of the mesh. Use for geo testing and etc.
	std::vector<face_t*> collision;

	// A triangulated representation of the face. This is what gets rendered. 
	//std::vector<face_t*> tris;

	// List of cut vertexes used by this part. Do not delete! We don't own these.
	std::vector<glm::vec3*> cutVerts;

	// List of faces produced by the cut. Most likely concave.
	std::vector<face_t*> faces;
};


struct textureMeshPartData_t
{
	glm::vec2 offset;
	glm::vec2 scale;
	texture_t texture;
};


// A mesh part is a face that is part of a larger mesh that holds more faces
struct meshPart_t : public face_t
{
	meshPart_t() { flags = FaceFlags::FF_MESH_PART; }
	~meshPart_t();

	// Neither the collision vector nor the tris vector should be used for true mesh editing, as they are constantly regenerated.

	// Convexified representation of the mesh part. Use for geo testing and etc.
	std::vector<face_t*> collision;

	// A triangulated representation of the face. This is what gets rendered.
	std::vector<face_t*> tris;

	// Data that gets populated when this mesh gets sliced. When this is not nullptr, it should take priority over our collisions and tris
	slicedMeshPartData_t* sliced = nullptr;

	// What mesh do we belong to
	mesh_t* mesh = nullptr;

	// Precomputed normal of the part. Generated by defineMeshPartFaces
	glm::vec3 normal;

	// Optional data for rendering this part's texture
	textureMeshPartData_t txData;
};

struct mesh_t
{
	~mesh_t();

	// The faces of this mesh
	std::vector<meshPart_t*> parts;
	
	// Not ordered! Do no depend on this!
	// Pointers so that adding doesn't delete the memory
	std::vector<glm::vec3*> verts;

	// Calculated from part aabbs
	//aabb_t aabb;

	// Transform of the mesh
	glm::vec3 origin;
};

// A mesh that can be sliced by other meshes
struct cuttableMesh_t : public mesh_t
{
	/*
	// These cut into this mesh without changing the real structure of it
	std::vector<mesh_t*> cutters;

	// We cut these meshes
	std::vector<mesh_t*> cutting;
	*/

	// Not ordered! Do no depend on this!
	// Pointers so that adding doesn't delete the memory
	// Added during cutting
	std::vector<glm::vec3*> cutVerts;
};


// Returns start of the points within mesh.verts
glm::vec3** addMeshVerts(mesh_t& mesh, glm::vec3* points, int pointCount);

// Does not add points to mesh! Only adds a face
meshPart_t* addMeshFace(mesh_t& mesh, glm::vec3** points, int pointCount);

void defineMeshPartFaces(meshPart_t& mesh);


aabb_t meshAABB(mesh_t& mesh);
void recenterMesh(mesh_t& mesh);
glm::vec3 faceCenter(face_t* face);
glm::vec3 faceNormal(face_t* face, glm::vec3* outCenter = nullptr);
glm::vec3 convexFaceNormal(face_t* face);
glm::vec3 vertNextNormal(vertex_t* vert);
unsigned int edgeLoopCount(vertex_t* sv);
inline unsigned int edgeLoopCount(halfEdge_t* sh) { return edgeLoopCount(sh->vert); }


void faceFromLoop(halfEdge_t* startEdge, face_t* faceToFill);


void cloneFaceInto(face_t* in, face_t* cloneOut);

inline meshPart_t* parentPart(face_t* f) { if (!f) return 0; if (f->flags & FaceFlags::FF_MESH_PART) return static_cast<meshPart_t*>(f); if (f->parent) return parentPart(f->parent); return 0; }
inline mesh_t* parentMesh(face_t* f) { if (!f) return 0; meshPart_t* part = parentPart(f); if (part) return part->mesh; return 0; }


// Bulks up an AABB if the point is outside
aabb_t addPointToAABB(aabb_t aabb, glm::vec3 point);

// This AABB will have it's max and min flipped and as FLT_MAX
// Useful for creating a new AABB
inline aabb_t invertedAABB() { return { {FLT_MAX,FLT_MAX,FLT_MAX}, {-FLT_MAX,-FLT_MAX,-FLT_MAX} }; }