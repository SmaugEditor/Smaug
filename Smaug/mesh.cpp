#include "mesh.h"
#include "raytest.h"
#include "containerutil.h"
#include <glm/geometric.hpp>

/*
Process that happens here

INPUT
________________________
|                      |
|       ________       |
|       |XXXXXX|       |
|       |XXXXXX|       |
|                      |
|______________________|

SLICE
________________________
|              |       |
|______________|       |
|       |XXXXXX|       |
|       |XXXXXX|_______|
|       |              |
|_______|______________|

TRIANGULATE

________________________
|\............ |\      |
|_____________\|  \    |
|\      |XXXXXX|    \  |
|  \    |XXXXXX|______\|
|    \  |\............ |
|______\|_____________\|


*/

template<typename T>
void swap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}

// For use where we know which side's going to end up larger
// Returns the new face
face_t* sliceMeshPartFaceUnsafe(meshPart_t& mesh, face_t* face, vertex_t* start, vertex_t* end)
{
	face_t* newFace = new face_t;
	newFace->meshPart = &mesh;
	mesh.faces.push_back(newFace);

	halfEdge_t* heStart = start->edge;
	halfEdge_t* heEnd   = end->edge;

	
	// Let's just let it keep end...
	// We're totally stealing start though!
	vertex_t* newEnd = new vertex_t{ end->vert, end->edge };

	newFace->verts.push_back(newEnd);
	newFace->sideCount = 1;

	// We need to steal from our old and add to our new
	for (halfEdge_t* he = heEnd; he != heStart; he = he->next)
	{
		printf("%p\n", (void*)(he));
		he->face = newFace;
		newFace->verts.push_back(he->vert);
		newFace->edges.push_back(he);
		newFace->sideCount++;
	}

	// Set up our new HE from start to end on the small side
	halfEdge_t* newHe = new halfEdge_t{ newEnd, nullptr, newFace, heEnd };
	halfEdge_t* prevStart = newFace->edges[newFace->edges.size()-1];
	prevStart->next = newHe;
	prevStart->vert->edge = newHe;
	newFace->edges.push_back(newHe);


	// Abysmal
	face->verts.clear();
	face->edges.clear();

	vertex_t* newStart = new vertex_t{ start->vert, heStart };
	face->verts.push_back(newStart);
	face->sideCount = 1;
	for (halfEdge_t* he = heStart; he != heEnd; he = he->next)
	{
		face->verts.push_back(he->vert);
		face->edges.push_back(he);
		face->sideCount++;
	}

	// New HE for our larger half
	halfEdge_t* newHeLarger = new halfEdge_t{ newStart, newHe, face, heStart };
	newHe->pair = newHeLarger;
	halfEdge_t* prevEnd = face->edges[face->edges.size() - 1];
	prevEnd->next = newHeLarger;
	prevEnd->vert->edge = newHeLarger;
	face->edges.push_back(newHeLarger);


	return newFace;
}

// This will subdivide a mesh's face from start to end
// It will return the new face
// This cut will only occur on ONE face
face_t* sliceMeshPartFace(meshPart_t& mesh, face_t* face, vertex_t* start, vertex_t* end)
{
	// No slice
	if (start == end || !start || !end || !face)
		return face;


	// Don't like this much...
	int between = 0;
	for(vertex_t* vert = start; vert != end; vert = vert->edge->vert)
	{
		between++;
	}

	// Start should always be the clockwise start of the smaller part
	if (between < face->edges.size() / 2.0f)
	{
		swap(start, end);
	}

	// Now that we know which side's larger, we can safely slice
	return sliceMeshPartFaceUnsafe(mesh, face, start, end);
}

void defineFace(face_t& face, CUArrayAccessor<glm::vec3*> vecs, int vecCount);
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
void addMeshFace(mesh_t& mesh, glm::vec3** points, int pointCount)
{
	meshPart_t* mp = new meshPart_t;
	mp->mesh = &mesh;
	mp->sideCount = pointCount;
	mesh.parts.push_back(mp);

	defineFace(*mp, points, pointCount);
}

void triangluateMeshPartFaces(meshPart_t& mesh)
{
	// As we'll be walking this, we wont want to walk over our newly created faces
	// Store our len so we only get to the end of the predefined faces
	size_t len = mesh.faces.size();
	for (size_t i = 0; i < len; i++)
	{
		face_t* face = mesh.faces[i];
		if (!face)
			continue;
		
		// Discard Tris
		if (face->edges.size() < 4)
			continue;

		// Center of face
		glm::vec3 center = { 0,0,0 };
		for (auto v : face->verts)
			center += *v->vert;
		center /= face->verts.size();

		// Get the norm of the face for later testing 
		// Not fond of this
		glm::vec3 faceNormal = { 0,0,0 };
		for (int i = 1; i < face->verts.size(); i++)
		{
			faceNormal += glm::cross((center - (*face->verts[i]->vert)), (*face->verts[i-1]->vert) - center);
		}
		faceNormal = glm::normalize(faceNormal);
		
		int nU, nV;
		findDominantAxis(faceNormal, nU, nV);

		// On odd numbers, we use start + 1, end instead of start, end - 1
		// Makes it look a bit like we're fitting quads instead of tris
		int alternate = 0;

		// In these loop, we'll progressively push the original face to become smaller and smaller
		while (face->verts.size() > 3)
		{
			// Vert 0 will become our anchor for all new faces to connect to
			vertex_t* v0;
			vertex_t* end;
			if (fmod(alternate, 2) == 1)
			{
				v0 = face->verts[1];
				end = face->verts[face->verts.size() - 1];
			}
			else
			{
				v0 = face->verts[0];
				end = face->verts[face->verts.size() - 2];
			}

			int attempts = 0;
			bool shift;
			do{
				shift = false;

				// Will this be convex?
				vertex_t* between = end->edge->vert;
				glm::vec3 edge1 = (*end->vert) - (*between->vert);
				glm::vec3 edge2 = (*between->vert) - (*v0->vert);
				glm::vec3 triNormal = glm::cross(edge1, edge2);

				float dot = glm::dot(triNormal, faceNormal);
				if (dot < 0)
					shift = true;
				else
				{
					// Dot was fine. Let's see if anything's in our new tri

					glm::vec3 u = tritod(*end->vert, *between->vert, *v0->vert, nU);
					glm::vec3 v = tritod(*end->vert, *between->vert, *v0->vert, nV);

					// I fear the cache misses...
					for (vertex_t* c = v0->edge->vert; c != end; c = c->edge->vert)
					{
						glm::vec3 ptt = *c->vert;
						if (testPointInTri(ptt, *end->vert, *between->vert, *v0->vert))//(ptt[nU], ptt[nV], u, v))
						{
							shift = true;
							break;
						}
					}
				}

				if (shift)
				{
					// Shift to somewhere it shouldn't be convex
					v0 = v0->edge->vert;
					end = end->edge->vert;
					attempts++;
				}
			}
			while (shift && attempts < 10);

			// Wouldn't it just be better to implement a quick version for triangulate? We're doing a lot of slices? Maybe just a bulk slicer?
			sliceMeshPartFaceUnsafe(mesh, face, v0, end);

			alternate++;
		};

		
	}

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
	face.sideCount = 0;

	// Let the face know our data
	face.sideCount = vecCount;

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
	for (auto v : mesh.faces)
		if(v) delete v;
	mesh.faces.clear();

	face_t* f = new face_t;
	mesh.faces.push_back(f);
	f->meshPart = &mesh;
	
	if (mesh.verts.size() == 0)
		return;

	// Only give it our verts
	C2DPYSkipArray<vertex_t, glm::vec3*> skip(mesh.verts.data(), mesh.verts[0]->vert, 0);
	defineFace(*f, skip, mesh.verts.size());


	triangluateMeshPartFaces(mesh);
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
	for (auto f : faces)
		delete f;
}

mesh_t::~mesh_t()
{
	for (auto p : parts)
		delete p;

	for (auto v : verts)
		delete v;
}
