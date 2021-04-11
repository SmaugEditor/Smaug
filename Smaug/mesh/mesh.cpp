#include "mesh.h"
#include "raytest.h"
#include "containerutil.h"
#include "utils.h"
#include <glm/geometric.hpp>


// Returns the unnormalized normal of a face
glm::vec3 faceNormal(face_t* face, glm::vec3* outCenter)
{
	// Not fond of this
	// TODO: We should probably cache this data and/or add a quick route for triangulated faces!
	
	glm::vec3 center = faceCenter(face);

	glm::vec3 faceNormal = { 0,0,0 };
	for (int i = 1; i < face->verts.size(); i++)
	{
		faceNormal += glm::cross((center - (*face->verts[i]->vert)), (*face->verts[i - 1]->vert) - center);
	}

	if (outCenter)
		*outCenter = center;

	return faceNormal;
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
void defineFace(face_t& face, CUArrayAccessor<glm::vec3*> vecs, int vecCount);
void addMeshFace(mesh_t& mesh, glm::vec3** points, int pointCount)
{
	meshPart_t* mp = new meshPart_t;
	mp->mesh = &mesh;
	mesh.parts.push_back(mp);

	defineFace(*mp, points, pointCount);
}


// Wires up all the HEs for this face
// Does not triangulate
void defineFace(face_t& face, CUArrayAccessor<glm::vec3*> vecs, int vecCount)
{
	// Clear out our old data
	for (auto v : face.verts)
		if (v) delete v;
	for (auto e : face.edges)
		if (e) delete e;
	face.verts.clear();
	face.edges.clear();
	
	// Populate our face with HEs
	vertex_t* lastVert = nullptr;
	halfEdge_t* lastHe = nullptr;
	for (int i = 0; i < vecCount; i++)
	{
		halfEdge_t* he = new halfEdge_t{};
		he->face = &face;
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
		face.edges.push_back(he);
		face.verts.push_back(v);
	}

	// Link up our last HE
	if (lastHe && lastVert)
	{
		lastHe->next = face.edges.front();
		lastHe->vert = face.verts.front();
	}
}

void defineMeshPartFaces(meshPart_t& mesh)
{
	// Clear out our old faces
	for (auto f : mesh.fullFaces)
		delete f;
	for (auto f : mesh.cutFaces)
		delete f;
	mesh.fullFaces.clear();
	mesh.cutFaces.clear();

	face_t* f = new face_t;
	mesh.fullFaces.push_back(f);
	f->meshPart = &mesh;
	
	if (mesh.verts.size() == 0)
		return;

	// Only give it our verts
	C2DPYSkipArray<vertex_t, glm::vec3*> skip(mesh.verts.data(), mesh.verts[0]->vert, 0);
	defineFace(*f, skip, mesh.verts.size());
	

}
// Terrible
glm::vec3*& vertVectorAccessor(void* vec, size_t i)
{
	std::vector<vertex_t*>* verts = (std::vector<vertex_t*>*)vec;
	return verts->at(i)->vert;
}

void cloneFaceInto(face_t* in, face_t* cloneOut)
{
	defineFace(*cloneOut, { (void*)&in->verts, &vertVectorAccessor }, in->verts.size());
}


aabb_t meshAABB(mesh_t& mesh)
{

	glm::vec3 max = { FLT_MIN, FLT_MIN, FLT_MIN };
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
	for (auto f : fullFaces)
		delete f;

	for (auto f : cutFaces)
		delete f;
}

mesh_t::~mesh_t()
{
	for (auto p : parts)
		delete p;

	for (auto v : verts)
		delete v;
}
