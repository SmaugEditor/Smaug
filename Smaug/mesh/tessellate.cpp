#include "tessellate.h"
#include "raytest.h"
#include "meshtest.h"
#include "sassert.h"

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
face_t* sliceMeshPartFaceUnsafe(meshPart_t& mesh, std::vector<face_t*>& faceVec, face_t* face, vertex_t* start, vertex_t* end)
{
	face_t* newFace = new face_t;
	newFace->meshPart = &mesh;
	faceVec.push_back(newFace);

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
face_t* sliceMeshPartFace(meshPart_t& mesh, std::vector<face_t*>& faceVec, face_t* face, vertex_t* start, vertex_t* end)
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
	return sliceMeshPartFaceUnsafe(mesh, faceVec, face, start, end);
}

// This will REBUILD THE FACE!
// The next TWO edges from stem will be deleted!
halfEdge_t* fuseEdges(face_t* face, vertex_t* stem)
{
	// Fuse stem and it's next edge together
	// Hate this. Make it better somehow...
	
	// Since something's already pointing to stem->edge, it's going to take the place of post->edge
	halfEdge_t* replacer = stem->edge;

	// Edge immediately after the fuse
	halfEdge_t* post = replacer->next->vert->edge;
	
	// Clear out stuff to be fused
	delete replacer->next->vert;
	delete replacer->next;
	delete replacer->vert;

	replacer->vert = post->vert;
	replacer->next = post->next;

	delete post;

	// Rebuild the face. Yuck
	face->verts.clear();
	face->edges.clear();
	faceFromLoop(replacer, face);

	return replacer;
}


/////////////////////////////
// Mesh face triangulation //
/////////////////////////////

// Clean this all up!!
// Takes in a mesh part and triangulates every face within the part
// TODO: This might need a check for parallel lines!
void triangluateMeshPartFaces(meshPart_t& mesh, std::vector<face_t*>& faceVec)
{
	// As we'll be walking this, we wont want to walk over our newly created faces
	// Store our len so we only get to the end of the predefined faces
	size_t len = faceVec.size();
	for (size_t i = 0; i < len; i++)
	{
		face_t* face = faceVec[i];
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

				// Will this be concave?
				vertex_t* between = end->edge->vert;
				glm::vec3 edge1 = (*end->vert) - (*between->vert);
				glm::vec3 edge2 = (*between->vert) - (*v0->vert);
				glm::vec3 triNormal = glm::cross(edge1, edge2);

				//SASSERT(triNormal.x != 0 || triNormal.y != 0 || triNormal.z != 0);

				// If we get a zero area tri, WHICH WE SHOULD NOT PASS IN OR CREATE, trim it off
				if (triNormal.x == 0 && triNormal.y == 0 && triNormal.z == 0)
				{
					//SASSERT(0);

					// Fuse v0 and end together 
					fuseEdges(face, end);

					goto escapeTri;
				}

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
					// Shift to somewhere it shouldn't be concave
					v0 = v0->edge->vert;
					end = end->edge->vert;
					attempts++;
				}
			}
			while (shift && attempts < 10);

			// Wouldn't it just be better to implement a quick version for triangulate? We're doing a lot of slices? Maybe just a bulk slicer?
			sliceMeshPartFaceUnsafe(mesh, faceVec, face, v0, end);

			escapeTri:
			alternate++;
		};

		
	}

}

// This function fits convex faces to the concave face
// It might be slow, bench it later
void convexifyMeshPartFaces(meshPart_t& mesh, std::vector<face_t*>& faceVec)
{
	halfEdge_t gapFiller;
	
	// Get the norm of the face for later testing 
	glm::vec3 faceNorm = faceNormal(&mesh);
	size_t len = faceVec.size();
	for (size_t i = 0; i < len; i++)
	{
		face_t* face = faceVec[i];
		if (!face)
			continue;


		startOfLoop:
		// Discard Tris, they're already convex
		if (face->edges.size() < 4)
			continue;

		vertex_t* vStart = face->verts.front(),
			    * convexStart = vStart,
			    * vert = vStart;

		do
		{
			vertex_t* between = vert->edge->vert;
			vertex_t* end = between->edge->vert;

			glm::vec3 edge1 = (*vert->vert) - (*between->vert);
			glm::vec3 edge2 = (*between->vert) - (*end->vert);
			glm::vec3 triNormal = glm::cross(edge1, edge2);


			if (triNormal.x == 0 && triNormal.y == 0 && triNormal.z == 0)
			{
				// We got a zero area tri! Yuck!!
				SASSERT(0);
				fuseEdges(face, vert);
				continue;
			}


			bool concave = false;
			float dot = glm::dot(triNormal, faceNorm);

			if (dot < 0)
			{
				// Uh oh! This'll make us concave!
				concave = true;
			}
			else
			{
				// Would trying to connect up to convex start make us concave?
				glm::vec3 bridge1 = (*end->vert) - (*convexStart->vert);
				glm::vec3 bridge2 = (*convexStart->vert) - (*convexStart->edge->vert->vert);
				glm::vec3 bridgeNormal = glm::cross(bridge1, bridge2);
				float bridgeDot = glm::dot(bridgeNormal, faceNorm);

				if (bridgeDot < 0)
					concave = true;
				else
				{
					bridge1 = (*between->vert) - (*end->vert);
					bridge2 = (*end->vert) - (*convexStart->vert);
					bridgeNormal = glm::cross(bridge1, bridge2);
					bridgeDot = glm::dot(bridgeNormal, faceNorm);
					if (bridgeDot < 0)
						concave = true;
				}

			}
			
			if(!concave)
			{
				// Hmm... Doesn't seem concave... Let's double check
				// Patch up the mesh and perform a point test
				
				// Shouldnt need to store between's edge's next...
				halfEdge_t* tempHE = end->edge;
				
				// Temp patch it
				halfEdge_t* cse = convexStart->edge;
				gapFiller.next = cse;
				gapFiller.vert = convexStart;

				end->edge = &gapFiller;
				between->edge->next = &gapFiller;
				
				// Loop over our remaining verts and check if in our convex fit
				for (vertex_t* ooc = tempHE->vert; ooc != convexStart; ooc = ooc->edge->vert)
				{
					if (pointInConvexLoop(convexStart, *ooc->vert))
					{
						// Uh oh concave!
						concave = true;
						break;
					}
				}

				// Undo our patch
				end->edge = tempHE;
				between->edge->next = tempHE;
			}

			if (concave)
			{
				// This shape's concave!
				// We'll need to cap off this shape at whatever we had last time...
				
				if (vert == convexStart)
				{
					// Shift forwards and try that one later
					convexStart = convexStart->edge->vert;
				}
				else
				{
					// Connect between and start
					sliceMeshPartFaceUnsafe(mesh, faceVec, face, between, convexStart);
					goto startOfLoop;
				}
			}

			vert = between;
		} while (vert->edge->vert->edge->vert != convexStart);
		
	}
}
