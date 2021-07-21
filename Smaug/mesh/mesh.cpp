#include "mesh.h"
#include "raytest.h"
#include "containerutil.h"
#include "utils.h"
#include <glm/geometric.hpp>

// Gets the normal of the *next* vert
glm::vec3 vertNextNormal(vertex_t* vert)
{
	vertex_t* between = vert->edge->vert;
	vertex_t* end = between->edge->vert;

	glm::vec3 edge1 = (*vert->vert) - (*between->vert);
	glm::vec3 edge2 = (*between->vert) - (*end->vert);
	
	return glm::cross(edge1, edge2);
}

// Returns the unnormalized normal of a face
glm::vec3 faceNormal(face_t* face, glm::vec3* outCenter)
{
	// Not fond of this
	// TODO: We should probably cache this data and/or add a quick route for triangulated faces!
	
	glm::vec3 center = faceCenter(face);

	if (outCenter)
		*outCenter = center;

	if (face->flags & FaceFlags::FF_CONVEX)
	{
		return convexFaceNormal(face);
	}

	glm::vec3 faceNormal = { 0,0,0 };

	vertex_t* vs = face->verts.front(), * l = vs, * v = l->edge->vert;
	do
	{
		faceNormal += glm::cross((center - (*v->vert)), (*l->vert) - center);
	} 	while (l != vs);

	return faceNormal;
}

glm::vec3 convexFaceNormal(face_t* face)
{
	return vertNextNormal(face->verts.front());
}


void faceFromLoop(halfEdge_t* startEdge, face_t* faceToFill)
{
	halfEdge_t* he = startEdge;
	do {
		faceToFill->edges.push_back(he);
		faceToFill->verts.push_back(he->vert);
		
		he->face = faceToFill;

		he = he->next;
	} while (he != startEdge);

}

/////////////////////
// Mesh management //
/////////////////////


// Adds vertices to a mesh for use in face definition
glm::vec3** addMeshVerts(mesh_t& mesh, glm::vec3* points, int pointCount)
{
	size_t start = mesh.verts.size();
	for (int i = 0; i < pointCount; i++)
	{
		glm::vec3* v = new glm::vec3(points[i]);
		mesh.verts.push_back(v);
	}
	return mesh.verts.data() + start;
}

// Creates and defines a new face within a mesh
void defineFace(face_t* face, CUArrayAccessor<glm::vec3*> vecs, int vecCount);
meshPart_t* addMeshFace(mesh_t& mesh, glm::vec3** points, int pointCount)
{
	meshPart_t* mp = new meshPart_t;
	mp->mesh = &mesh;
	mp->txData = { {0,0},{0.125,0.125},INVALID_TEXTURE };
	mesh.parts.push_back(mp);

	defineFace(mp, points, pointCount);

	return mp;
}


// Wires up all the HEs for this face
// Does not triangulate
void defineFace(face_t* face, CUArrayAccessor<glm::vec3*> vecs, int vecCount)
{
	// Clear out our old data
	for (auto v : face->verts)
		if (v) delete v;
	for (auto e : face->edges)
		if (e) delete e;
	face->verts.clear();
	face->edges.clear();
	
	// Populate our face with HEs
	vertex_t* lastVert = nullptr;
	halfEdge_t* lastHe = nullptr;
	for (int i = 0; i < vecCount; i++)
	{
		// HE stems out of V

		halfEdge_t* he = new halfEdge_t{};
		he->face = face;
		he->flags |= EdgeFlags::EF_OUTER;
		vertex_t* v = new vertex_t{ vecs[i], he };

		// Should never be a situation where these both arent null or something
		// If one is null, something's extremely wrong
		if (lastHe && lastVert)
		{
			lastHe->next = he;
			lastHe->vert = v;
		}

		lastHe = he;
		lastVert = v;
		face->edges.push_back(he);
		face->verts.push_back(v);
	}

	// Link up our last HE
	if (lastHe && lastVert)
	{
		lastHe->next = face->edges.front();
		lastHe->vert = face->verts.front();
	}
}

void defineMeshPartFaces(meshPart_t& mesh)
{
	// Clear out our old faces
	if (mesh.sliced)
	{
		delete mesh.sliced;
		mesh.sliced = nullptr;
	}
	for (auto f : mesh.tris)
		delete f;
	for (auto f : mesh.collision)
		delete f;
	mesh.collision.clear();
	mesh.tris.clear();

	if (mesh.verts.size() == 0)
		return;

	// Create a face clone of the part
	face_t* f = new face_t;
	cloneFaceInto(&mesh, f);
	f->parent = &mesh;
	f->flags = FaceFlags::FF_NONE;
	mesh.collision.push_back(f);
	
	// Mark our edges
	for (auto e : f->edges)
		e->flags |= EdgeFlags::EF_PART_EDGE;

	// Compute our normal
	mesh.normal = glm::normalize(faceNormal(&mesh));

}
// Terrible
glm::vec3*& vertVectorAccessor(void* vec, size_t i)
{
	std::vector<vertex_t*>* verts = (std::vector<vertex_t*>*)vec;
	return verts->at(i)->vert;
}

void cloneFaceInto(face_t* in, face_t* cloneOut)
{
	// Clear out our old data
	for (auto v : cloneOut->verts)
		if (v) delete v;
	for (auto e : cloneOut->edges)
		if (e) delete e;
	cloneOut->verts.clear();
	cloneOut->edges.clear();

	vertex_t*   lastCloneVert = nullptr;
	halfEdge_t* lastCloneEdge = nullptr;

	vertex_t* vs = in->verts.front(), *v = vs;
	do
	{
		// Clone this vert and the edge that stems out of it
		halfEdge_t* ch = new halfEdge_t;
		ch->face = cloneOut;
		ch->flags = v->edge->flags;
		vertex_t* cv = new vertex_t{v->vert, ch};
		
		if (lastCloneVert)
		{
			lastCloneEdge->next = ch;
			lastCloneEdge->vert = cv;
		}

		cloneOut->verts.push_back(cv);
		cloneOut->edges.push_back(ch);

		lastCloneVert = cv;
		lastCloneEdge = ch;

		v = v->edge->vert;
	} while (v != vs);

	if (lastCloneEdge)
	{
		lastCloneEdge->next = cloneOut->edges.front();
		lastCloneEdge->vert = cloneOut->verts.front();
	}

	cloneOut->flags = in->flags;
	cloneOut->parent = in->parent;
}

aabb_t addPointToAABB(aabb_t aabb, glm::vec3 point)
{
	if (point.x > aabb.max.x)
		aabb.max.x = point.x;
	if (point.y > aabb.max.y)
		aabb.max.y = point.y;
	if (point.z > aabb.max.z)
		aabb.max.z = point.z;

	if (point.x < aabb.min.x)
		aabb.min.x = point.x;
	if (point.y < aabb.min.y)
		aabb.min.y = point.y;
	if (point.z < aabb.min.z)
		aabb.min.z = point.z;

	return aabb;
}


unsigned int edgeLoopCount(vertex_t* sv)
{
	unsigned int i = 0;
	vertex_t* v = sv;
	do
	{
		i++;
		v = v->edge->vert;
	} while (v != sv);
	return i;
}

aabb_t meshAABB(mesh_t& mesh)
{

	glm::vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	glm::vec3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
	for (auto v : mesh.verts)
	{
		if (v->x > max.x)
			max.x = v->x;
		if (v->y > max.y)
			max.y = v->y;
		if (v->z > max.z)
			max.z = v->z;

		if (v->x < min.x)
			min.x = v->x;
		if (v->y < min.y)
			min.y = v->y;
		if (v->z < min.z)
			min.z = v->z;
	}

	return { min,max };
}

void recenterMesh(mesh_t& mesh)
{
	// Recenter the origin
	glm::vec3 averageOrigin = glm::vec3(0, 0, 0);
	for(auto v : mesh.verts)
	{
		averageOrigin += *v;
	}
	averageOrigin /= mesh.verts.size();

	// Keep it on whole numbers
	averageOrigin = { (int)averageOrigin.x, (int)averageOrigin.y, (int)averageOrigin.z };

	// Shift the vertexes
	mesh.origin += averageOrigin;
	for (auto v : mesh.verts)
	{
		*v -= averageOrigin;
	}
}

glm::vec3 faceCenter(face_t* face)
{
	// Center of face
	glm::vec3 center = { 0,0,0 };
	for (auto v : face->verts)
		center += *v->vert;
	center /= face->verts.size();

	return center;
}

face_t::~face_t()
{
	for (auto e : edges)
		delete e;
	for (auto v : verts)
		delete v;
}

meshPart_t::~meshPart_t()
{
	if(sliced)
		delete sliced;

	for (auto f : collision)
		delete f;

	for (auto f : tris)
		delete f;
}

mesh_t::~mesh_t()
{
	for (auto p : parts)
		delete p;

	for (auto v : verts)
		delete v;
}

slicedMeshPartData_t::~slicedMeshPartData_t()
{
	for (auto f : collision)
		delete f;

	for (auto f : faces)
		delete f;
}
