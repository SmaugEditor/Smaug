#include "mesh.h"
#include "raytest.h"
#include "containerutil.h"
#include <glm/geometric.hpp>

// Move this to util or something?
template<typename T>
void swap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}

///////////////////////
// Mesh face slicing //
///////////////////////

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

	// We need to steal from our old and add to our new
	for (halfEdge_t* he = heEnd; he != heStart; he = he->next)
	{
		he->face = newFace;
		newFace->verts.push_back(he->vert);
		newFace->edges.push_back(he);
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
	for (halfEdge_t* he = heStart; he != heEnd; he = he->next)
	{
		face->verts.push_back(he->vert);
		face->edges.push_back(he);
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

// Returns the unnormalized normal of a face
glm::vec3 faceNormal(face_t* face, glm::vec3* outCenter)
{
	// Not fond of this

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

/////////////////////////////
// Mesh face triangulation //
/////////////////////////////

// Clean this all up!!
// Takes in a mesh part and triangulates every face within the part
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

		
		// Get the norm of the face for later testing 
		glm::vec3 faceNorm = faceNormal(face);
		
		//int nU, nV;
		//findDominantAxis(faceNorm, nU, nV);

		// On odd numbers, we use start + 1, end instead of start, end - 1
		// Makes it look a bit like we're fitting quads instead of tris
		int alternate = 0;

		// In these loop, we'll progressively push the original face to become smaller and smaller
		while (face->verts.size() > 3)
		{
			// Vert 0 will become our anchor for all new faces to connect to
			vertex_t* end = face->verts[face->verts.size() - 1 - fmod(alternate, 2)];
			vertex_t* v0 = end->edge->next->vert;

			int attempts = 0;
			bool shift;
			do{
				shift = false;

				// Will this be convex?
				vertex_t* between = end->edge->vert;
				glm::vec3 edge1 = (*end->vert) - (*between->vert);
				glm::vec3 edge2 = (*between->vert) - (*v0->vert);
				glm::vec3 triNormal = glm::cross(edge1, edge2);

				float dot = glm::dot(triNormal, faceNorm);
				if (dot < 0)
					shift = true;
				else
				{
					// Dot was fine. Let's see if anything's in our new tri

					//glm::vec3 u = tritod(*end->vert, *between->vert, *v0->vert, nU);
					//glm::vec3 v = tritod(*end->vert, *between->vert, *v0->vert, nV);

					// I fear the cache misses...
					for (vertex_t* c = v0->edge->vert; c != end; c = c->edge->vert)
					{
						glm::vec3 ptt = *c->vert;
						if (testPointInTriNoEdges(ptt, *end->vert, *between->vert, *v0->vert))//(ptt[nU], ptt[nV], u, v))
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

/////////////////////////////
// Mesh face shape cutting //
/////////////////////////////

// Cuts another face into a face using the cracking method
//
// Input
//  ________
// |        |
// |        |           /\
// |        |    +     /  \
// |        |         /____\
// |________|
//
// Output
//  ________
// |        |
// |   /\   |
// |  /  \  |
// | /____\ |
// |_______\|  < outer face is "cracked" and shaped around the cut
//               ready to be triangulated for drawing
//

// Mesh part must belong to a cuttable mesh!
// Cutter must have an opposing normal, verts are counter-clockwise
void opposingFaceCrack(cuttableMesh_t& mesh, meshPart_t* part, meshPart_t* cutter)
{
	// This fails if either parts have < 3 verts
	if (part->verts.size() < 3 || cutter->verts.size() < 3)
		return;
	std::vector<vertex_t*> cutterVerts = cutter->verts;

	// Find the closest two points
	float distClosest = FLT_MAX;
	vertex_t* v1Closest = nullptr;
	vertex_t* v2Closest = nullptr;
	int v2Index = 0;

	glm::vec3 cutterToLocal = +cutter->mesh->origin - mesh.origin;
	
	// We don't want to break this face's self representation
	// We're going to be editing child face #0
	// This means cracking *must* be done before triangulation
	// Which really sucks cause point testing is going to be waaay harder
	// We might want a representation below the mesh face for this...
	face_t* target = part->faces[0];
	
	// This might have horrible performance...
	for (auto v1 : target->verts)
	{
		// Skip out of existing cracks
		if (v1->edge->pair != nullptr)
			continue;

		int v2i = 0;
		for (auto v2 : cutterVerts)
		{
			glm::vec3 delta = *v1->vert - (*v2->vert + cutterToLocal);

			// Cheap dist
			// We don't need sqrt for just comparisons
			float dist = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
			if (dist < distClosest)
			{
				distClosest = dist;
				v1Closest = v1;
				v2Closest = v2;
				v2Index = v2i;
			}
			v2i++;
		}
	}


	// Let's start by tracking all of our new found points
	// aaand transforming them into our local space
	int startSize = mesh.cutVerts.size();
	for (auto v : cutterVerts)
	{
		glm::vec3* vec = new glm::vec3();
		*vec = *v->vert + cutterToLocal;
		mesh.cutVerts.push_back(vec);
	}
	glm::vec3** cutVerts = mesh.cutVerts.data() + startSize;
	//long long transformPointerToLocal = cutterVerts - cutter->verts.data();

	// Since we checked the len earlier, v1 and v2 should exist...
	
	// Start the crack into the face
	halfEdge_t* crackIn = new halfEdge_t{nullptr, nullptr, target, nullptr};
	target->edges.push_back(crackIn);


	vertex_t* curV = new vertex_t{cutVerts[v2Index]};
	crackIn->vert = curV;
	target->verts.push_back(curV);

	// Now we need to recreate our cutting mesh within this mesh
	halfEdge_t* curHE = crackIn;

	halfEdge_t* v2Edge = v2Closest->edge;
	
	halfEdge_t* otherHE = v2Edge;
	do
	{
		halfEdge_t* nHE = new halfEdge_t{};
		nHE->face = target;
		target->edges.push_back(nHE);

		curHE->next = nHE;
		curV->edge = nHE;

		// Don't like this!
		int offset = std::find(cutterVerts.begin(), cutterVerts.end(), otherHE->vert) - cutterVerts.begin();

		curV = new vertex_t{ cutVerts[offset] };
		nHE->vert = curV;
		target->verts.push_back(curV);

		curHE = nHE;

		otherHE = otherHE->next;
	} while (otherHE != v2Edge);


	

	// We need to crack v1 and v2 into two verts
	vertex_t* v1Out = new vertex_t{ v1Closest->vert, v1Closest->edge };
	target->verts.push_back(v1Out);

	// v1's going to need a new HE that points to v2
	halfEdge_t* crackOut = new halfEdge_t{ v1Out, crackIn, target, v1Out->edge };
	curHE->next = crackOut;
	curV->edge = crackOut;
	crackIn->pair = crackOut;
	target->edges.push_back(crackOut);

	// Don't like this
	// Walk and find what leads up to v1Closest
	halfEdge_t* preHE;
	for (preHE = v1Closest->edge; preHE->vert != v1Closest; preHE = preHE->next);
	preHE->next = crackIn;
	v1Closest->edge = crackIn;
}

void applyCuts(cuttableMesh_t& mesh)
{
	// Totally lame!
	// For now, we're just going to cut faces opposing each other...


	// Clear out our old cut verts
	for (auto v : mesh.cutVerts)
		delete v;
	mesh.cutVerts.clear();

	if (mesh.cutters.size() == 0)
		return;

	// Precompute the normals
	struct precomputed_t
	{
		glm::vec3 norm, center;
	};
	precomputed_t* precomp = new precomputed_t[mesh.parts.size()];
	for (int i = 0; auto p : mesh.parts)
	{
		precomp[i].norm = glm::normalize(faceNormal(p, &precomp[i].center));
		precomp[i].center += mesh.origin; // Offset all by origin
		i++;
	}
	
	for (auto cutter : mesh.cutters)
	{
		for (auto cutP : cutter->parts)
		{
			glm::vec3 cutCenter;
			glm::vec3 cutNorm = glm::normalize(faceNormal(cutP, &cutCenter));
			cutCenter += cutter->origin;

			// Iterate over faces and check if we have ones with opposing norms
			for (int i = 0; auto p : mesh.parts)
			{
				precomputed_t& pc = precomp[i];
				i++;
				
				glm::vec3 centerDiff = pc.center - cutCenter;

				// I's so tired. Review this in the morning
				centerDiff *= pc.norm;

				// Do we share an axis?
				if (glm::length(centerDiff) < 0.1f)
				{
					// Are our norms opposing?
					if (glm::dot(cutNorm, pc.norm) < 0)
					{
						opposingFaceCrack(mesh, p, cutP);
					}
				}

			}
		}

	}
	delete[] precomp;

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
