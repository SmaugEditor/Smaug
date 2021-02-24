#include "mesh.h"
#include "raytest.h"
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
face_t* sliceShapeFaceUnsafe(cutttableShape_t& shape, face_t* face, vertex_t* start, vertex_t* end)
{
	face_t* newFace = new face_t;
	newFace->meshPart = &shape;
	shape.faces.push_back(newFace);

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

// This will subdivide a shape's face from start to end
// It will return the new face
// This cut will only occur on ONE face
face_t* sliceShapeFace(cutttableShape_t& shape, face_t* face, vertex_t* start, vertex_t* end)
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
		swap(start,    end);
	}

	// Now that we know which side's larger, we can safely slice
	return sliceShapeFaceUnsafe(shape, face, start, end);
}

void triangluateShapeFaces(cutttableShape_t& shape)
{
	// As we'll be walking this, we wont want to walk over our newly created faces
	// Store our len so we only get to the end of the predefined faces
	size_t len = shape.faces.size();
	for (size_t i = 0; i < len; i++)
	{
		face_t* face = shape.faces[i];
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
		
		// On odd numbers, we use start + 1, end instead of start, end - 1
		// Makes it look a bit like we're fitting quads instead of tris
		int alternate = 0;

		// In these loop, we'll progressively push the original face to become smaller and smaller
		while(face->verts.size() > 3)
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


			// Will this be convex?
			vertex_t* between = face->verts[face->verts.size() - 1];
			glm::vec3 edge1 = (*v0->vert) - (*between->vert);
			glm::vec3 edge2 = (*between->vert) - (*end->vert);
			glm::vec3 triNormal = -glm::cross(edge1, edge2);

			float dot = glm::dot(triNormal, faceNormal);
			if (dot < 0)
			{
				// Shift to somewhere it shouldn't be convex
				v0 = v0->edge->vert;
				end = end->edge->vert;
			}

			// Wouldn't it just be better to implement a quick version for triangulate? We're doing a lot of slices? Maybe just a bulk slicer?
			sliceShapeFaceUnsafe(shape, face, v0, end);

			alternate++;
		};

		
	}

}

void defineShape(cutttableShape_t& shape)
{
	// Clear out our old faces
	for (auto v : shape.faces)
		if(v) delete v;
	shape.faces.clear();

	face_t* f = new face_t;
	shape.faces.push_back(f);

	// Let the face know our data
	f->sideCount = shape.verts.size();
	f->meshPart = &shape;

	// Populate our face with HEs
	vertex_t* lastVert = nullptr;
	halfEdge_t* lastHe = nullptr;
	for (glm::vec3& vert : shape.verts)
	{
		halfEdge_t* he = new halfEdge_t{};
		he->face = f;
		vertex_t* v = new vertex_t{ &vert, he };

		// Should never be a situation where these both arent null or something
		// If one is null, something's extremely wrong
		if (lastHe && lastVert)
		{
			lastHe->next = he;
			lastHe->vert = v;
		}

		lastHe = he;
		lastVert = v;
		f->edges.push_back(he);
		f->verts.push_back(v);
	}

	// Link up our last HE
	if (lastHe && lastVert)
	{
		lastHe->next = f->edges.front();
		lastHe->vert = f->verts.front();
	}

	//triangluateShapeFaces(shape);
}

void slicePlane(face_t* f, face_t** )
{

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
