#include "mesh.h"
#include "raytest.h"
#include "containerutil.h"
#include "utils.h"
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

void faceFromLoop(halfEdge_t* startEdge, face_t* faceToFill);
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

				// Will this be convex?
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
					// Hate this
					delete end->edge;
					delete between->edge;

					
					halfEdge_t* he = v0->edge;
					do
					{
						he = he->next;
					} while (he->vert != end);
					he->next = v0->edge;
					he->vert = v0;
					delete between;
					delete end;

					face->verts.clear();
					face->edges.clear();
					faceFromLoop(he, face);

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
					// Shift to somewhere it shouldn't be convex
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
	std::vector<vertex_t*>& cutterVerts = cutter->verts;

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
	face_t* target = part->cutFaces[0];
	
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


struct snipIntersect_t
{
	bool entering;
	float cutterT;
	glm::vec3 intersect;
	vertex_t* vert;
	halfEdge_t* edge;
};

struct inCutFace_t : public face_t
{
	int cullDepth = 0; // How many times has this been marked for culling?
};

// Pair of edges that can be dragged from the inner vert
// Could just deref out of right I guess...
struct draggable_t
{
	halfEdge_t* into;  // leads into vert
	vertex_t* vert;    // leads into left
	halfEdge_t* outof; // leads out of vert
};

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

// Intersect must exist within cut points!
draggable_t splitHalfEdgeAtPoint(halfEdge_t* he, glm::vec3* intersect)
{
	halfEdge_t* splitEdge = new halfEdge_t;
	vertex_t* splitVert = new vertex_t;

	// Split stems out of the vert
	splitVert->edge = splitEdge;
	splitVert->vert = intersect;

	// Split comes after original
	splitEdge->next = he->next;
	splitEdge->vert = he->vert;

	he->next = splitEdge;
	// splitVert stems OUT of he
	he->vert = splitVert;

	// Share the pair for now
	splitEdge->pair = he->pair;

	// Register up the parts
	face_t* face = he->face;
	face->edges.push_back(splitEdge);
	face->verts.push_back(splitVert);
	splitEdge->face = face;

	return { he, splitVert, splitEdge };
}

// Intersect must exist within cut points!
// Splits the other side too!
// Returns the new edge split off of the input
halfEdge_t* splitEdgeAtPoint(halfEdge_t* he, glm::vec3* intersect)
{
	halfEdge_t* pair = he->pair;

	if (pair)
	{
		// We need to alternate the pairs!
		//
		// O--.-->
		//    X      swap the pairs
		// <--.--O

		halfEdge_t* splitOrig = splitHalfEdgeAtPoint(he, intersect).outof;
		halfEdge_t* splitPair = splitHalfEdgeAtPoint(pair, intersect).outof;

		pair->pair = splitOrig;
		he->pair = splitPair;

		return splitOrig;
	}
	else
	{
		return splitHalfEdgeAtPoint(he, intersect).outof;

	}


}

halfEdge_t* addEdge(face_t* face)
{
	halfEdge_t* edge = new halfEdge_t;
	edge->face = face;
	face->edges.push_back(edge);
	return edge;
}

// Cracks a vertex in two and returns the opposing vert
// New vertex is floating! Requires linking to the list!
vertex_t* splitVertex(vertex_t* in, face_t* face)
{
	vertex_t* vert = new vertex_t;

	vert->edge = in->edge;
	vert->vert = in->vert;

	face->verts.push_back(vert);

	return vert;
}



// Takes in a split from splitHalfEdgeAtPoint
// Drags the split out to the selected position
//
//     | ^
//     | |
//  L  v |  R
//  <---.<---
//
// R is the original after a split and L is the new
void dragEdgeInto(draggable_t draggable, draggable_t target)
{
	halfEdge_t* edgeLeft = draggable.outof;
	halfEdge_t* edgeRight = draggable.into;
	vertex_t* vert = draggable.vert;

	face_t* face = edgeLeft->face;

	// Crack the target so intoTarget can stick into it
	// intoVert will stick out of target
	vertex_t* splitTarget = splitVertex(target.vert, face);
	// Crack the vert so intoTarget can stick out of it
	// intoVert will stick into vert
	vertex_t* splitVert = splitVertex(vert, face);
	
	// Points out of the right hand split
	halfEdge_t* intoVert = addEdge(face);
	// Points in to the left hand split
	halfEdge_t* intoTarget = addEdge(face);

	// Link the pairs as this is a full edge
	intoVert->pair = intoTarget;
	intoTarget->pair = intoVert;

	// Link up intoVert
	target.into->next = intoVert;
	target.vert->edge = intoVert;
	intoVert->vert = vert;
	intoVert->next = edgeLeft;

	// Link up intoTarget
	edgeRight->next = intoTarget;
	edgeRight->vert = splitVert;
	splitVert->edge = intoTarget;
	intoTarget->vert = splitTarget;
	intoTarget->next = splitTarget->edge;

}
draggable_t dragEdge(draggable_t draggable, glm::vec3* point)
{
	halfEdge_t* edgeLeft = draggable.outof;
	halfEdge_t* edgeRight = draggable.into;
	vertex_t* vert = draggable.vert;

	face_t* face = edgeLeft->face;

	// Crack the vert so edgeLeft can stick out of it
	vertex_t* splitVert = splitVertex(vert, face);

	// Points out of the right hand split
	halfEdge_t* out = addEdge(face);
	// Points in to the left hand split
	halfEdge_t* in = addEdge(face);
	
	// Link the pairs as this is a full edge
	in->pair = out;
	out->pair = in;

	// In needs a vert to stem out of and for out to lead into 
	vertex_t* inStem = new vertex_t;
	inStem->edge = in;
	inStem->vert = point;
	face->verts.push_back(inStem);

	// Link up the right side into out
	// edgeRight's vert stays the same, but the vert now points into out
	vert->edge = out;
	edgeRight->next = out;

	// Link out into in
	out->vert = inStem;
	out->next = in;

	// Link up in into left
	in->next = edgeLeft;
	in->vert = splitVert;

	return { out, inStem, in };
}


// Roll around the cutter
// While rolling, check if any lines intersect
// If a line intersects, and we're leaving, stop adding data until we re-enter the face
// If a line intersects, and we're entering, keep adding data until we're out
// Line leaves when dot of cross of intersect and face norm is < 0
// TODO: Optimize this!
void faceSnips(cuttableMesh_t& mesh, mesh_t& cuttingMesh, meshPart_t* part, meshPart_t* cutter)
{
	glm::vec3 meshOrigin        = mesh.origin;
	glm::vec3 cuttingMeshOrigin = cuttingMesh.origin;
	glm::vec3 partNorm = faceNormal(part);

	// Are we slicing into the face?
	bool cutting = false;

	// Last edit 
	draggable_t drag{ 0,0,0 };

	// We need to walk either into or out of the mesh
	// 

	bool invalidateSkipOver = false;

	// Loop the part
	// As we slice, more faces will be added. 
	// Those faces will get chopped after the first face is done
	vertex_t* cvStart = cutter->verts.front();
	vertex_t* cv = cvStart;
	do
	{
		// We need to store our intersections so we can sort by distance
		std::vector<snipIntersect_t> intersections;

		bool sharedLine = false;
		for (int i = 0; i < part->cutFaces.size(); i++)
		{
			face_t* face = part->cutFaces[i];

			vertex_t* pvStart = face->verts.front();
			vertex_t* pv = pvStart;
			do
			{
				// Dragged edges have pairs, normal cloned edges don't
				// TODO: improve determination of how to skip dragged edges
				if (pv->edge->pair)
				{
					pv = pv->edge->vert;
					continue;
				}

				// The line we're cutting
				glm::vec3 partStem = *pv->vert;
				glm::vec3 partEndLocal = *pv->edge->vert->vert;
				line_t pL = { partStem + meshOrigin, partEndLocal - partStem };

				// The line we're using to cut with
				glm::vec3 cutterStem = *cv->vert;
				glm::vec3 cutterEndLocal = *cv->edge->vert->vert;
				line_t cL = { cutterStem + cuttingMeshOrigin, cutterEndLocal - cutterStem };

				
				// If this line is parallel and overlapping with anything, it's not allowed to do intersections!
				glm::vec3 cross = glm::cross(cL.delta, pL.delta);
				if (!sharedLine && (cross.x == 0 && cross.y == 0 && cross.z == 0))
				{
					glm::vec3 absPartStem = partStem + meshOrigin;
					glm::vec3 absCutterStem = cutterStem + cuttingMeshOrigin;

					// If our lines are on top of eachother, we need to void some tests...
					glm::vec3 stemDir = glm::cross(cL.delta, absPartStem - absCutterStem);
					if (stemDir.x < 0.0001 && stemDir.y < 0.0001 && stemDir.z < 0.0001)
					{
						glm::vec3 absPartEnd = partEndLocal + meshOrigin;
						glm::vec3 absCutterEnd = cutterEndLocal + cuttingMeshOrigin;

						// Are we actually within this line though?
						/*
							end = stem + delta * m
							(e - s) . d = m;
						*/

						float lpow = pow(glm::length(pL.delta), 2);
						float mCutterStem = glm::dot(absCutterStem - absPartStem, pL.delta) / lpow;
						float mCutterEnd = glm::dot(absCutterEnd - absPartStem, pL.delta) / lpow;

						// Since this is in terms of pL.delta, a mag of 1 is = to part end and 0 = part stem
						sharedLine = rangeInRange<false, true>(0, 1, mCutterStem, mCutterEnd);
						
					}

					// With a cross of 0, the line test will never work. We'll skip it now
					pv = pv->edge->vert;
					continue;
				}

				testLineLine_t t = testLineLine(cL, pL, 0.001f);
				if (t.hit)
				{
					bool entering = glm::dot(glm::cross(pL.delta, cL.delta), partNorm) >= 0;
					if (entering)
					{
						// If we're entering, we want to ignore perfect intersections where the end of this cut line *just* hits a part line
						// It's 1 if it's a full filled delta
						if (closeTo(t.t1, 1))
						{
							// Drop it
							pv = pv->edge->vert;
							continue;
						}
					}
					else
					{
						// If we're exiting, we want to ignore perfect intersections where the stem of this cut line *just* hits a part line
						// It's 0 if it's a next to the stem
						if (closeTo(t.t1, 0))
						{
							// Drop it
							pv = pv->edge->vert;
							continue;
						}
					}
					// Store the intersection for later
					snipIntersect_t si
					{
						.entering = entering,
						.cutterT = t.t1,
						.intersect = t.intersect,
						.vert = pv,
						.edge = pv->edge,
					};
					intersections.push_back(si);
				}

				pv = pv->edge->vert;
			} while (pv != pvStart);
		}

		// Now that we have all of our intersections, we need to sort them all.
		// Sort by closest to stem
		std::qsort(intersections.data(), intersections.size(), sizeof(snipIntersect_t), [](void const* a, void const* b) -> int {
			const snipIntersect_t* siA = static_cast<const snipIntersect_t*>(a);
			const snipIntersect_t* siB = static_cast<const snipIntersect_t*>(b);
			if (siA->cutterT < siB->cutterT)
				return -1;
			else if (siA->cutterT > siB->cutterT)
				return 1;
			else
				return 0;
			});

		// Did we intersect?
		if (intersections.size())
		{
			for (int i = 0; i < intersections.size(); i++)
			{
				snipIntersect_t& si = intersections[i];

				// I would rather do this as a preprocess, but we can't determine when sharedLine will be true...
				if (sharedLine)
				{
					// If we're sharing a line, we can't be cutting into endpoints!
					if (glm::distance(si.intersect, *si.vert->vert + meshOrigin) < 0.001
						|| glm::distance(si.intersect, *si.edge->vert->vert + meshOrigin) < 0.001)
					{
						invalidateSkipOver = si.entering;
						// We're cutting an endpoint! Skip it!
						continue;
					}
				}

				if (si.entering)
				{
					// Starts a cut
					cutting = true;

					// Split the intersected edge
					glm::vec3* point = new glm::vec3();
					*point = si.intersect - meshOrigin;
					mesh.cutVerts.push_back(point);
					drag = splitHalfEdgeAtPoint(si.edge, point);

					// If we're at the end of our intersections, then we can just drag this all the way out.
					// There's no end cap right now
					if (intersections.size() - 1 == i)
					{
						// Drag to the end of this edge
						glm::vec3* end = new glm::vec3();
						*end = *cv->edge->vert->vert + cuttingMeshOrigin - meshOrigin + glm::vec3(0, -0.1, 0);
						mesh.cutVerts.push_back(end);
						drag = dragEdge(drag, end);
					}
				}
				else
				{
					// Caps off a cut
					if (cutting)
					{
						// Split at intersection
						glm::vec3* point = new glm::vec3();
						*point = si.intersect - meshOrigin;
						mesh.cutVerts.push_back(point);
						draggable_t target = splitHalfEdgeAtPoint(si.edge, point);

						// Drag the edge into the other side
						dragEdgeInto(drag, target);

						// We should be done cutting this part now.
						cutting = false;

						// Split our edit into two different faces.
						// The one we think will get culled will be new
						
						// Clear ourselves so we can make sure we just have the data we need
						inCutFace_t* owner = (inCutFace_t*)target.outof->face;
						owner->cullDepth = 0;
						owner->edges.clear();
						owner->verts.clear();

						inCutFace_t* other = new inCutFace_t;
						other->cullDepth = -1;
						other->meshPart = part;
						part->cutFaces.push_back(other);

						faceFromLoop(target.outof, owner);
						faceFromLoop(target.into, other);

					}
					else
					{
						// If we entered this via a shared line, we don't want to shift our cvStart
						if (!invalidateSkipOver)
						{
							// We're exciting now?
							// When not cutting?
							// Seems like we need to loop more!
							// Try to roll out of the mesh so another point can cut into it.
							cv = cv->edge->vert;
							cvStart = cv;
						}
						invalidateSkipOver = false;
					}

				}

				// Loop the intersections again, incase we sliced through two parts with one edge
			}
		}
		else
		{
			// We didn't intersect, but we might have to drag a cut

			// Continues a cut
			if (cutting)
			{
				// Create a new point within our edit space
				glm::vec3 editPoint = *cv->edge->vert->vert + cuttingMeshOrigin - meshOrigin;
				glm::vec3* newEditPoint = new glm::vec3();
				*newEditPoint = editPoint;
				mesh.cutVerts.push_back(newEditPoint);

				// Drag the previous cut to our new location
				drag = dragEdge(drag, newEditPoint);
			}
		}

		cv = cv->edge->vert;
	} while (cv != cvStart);
	// Loop back to the top and try on the newly added faces

	// If this is one, we failed to cut anything! Try a face crack?
	if (part->cutFaces.size() == 1)
	{
		opposingFaceCrack(mesh, part, cutter);
	}
	else
	{
		std::vector<face_t*> cleanFaces;
		// Cull off faces with < 0 depth
		for (int i = 0; i < part->cutFaces.size(); i++)
		{
			inCutFace_t* f = static_cast<inCutFace_t*>(part->cutFaces[i]);
			if (f->cullDepth < 0)
			{
				// Unlink twins
				for (auto e : f->edges)
					if (e->pair)
						e->pair->pair = nullptr;

				delete f;
			}
			else
				cleanFaces.push_back(f);
		}
		part->cutFaces = cleanFaces;
	}
	
}


template<typename F = face_t>
F* cloneFace(face_t* f);

void applyCuts(cuttableMesh_t& mesh)
{
	// Totally lame!
	// For now, we're just going to cut faces opposing each other...


	// Clear out our old cut verts
	for (auto v : mesh.cutVerts)
		delete v;
	mesh.cutVerts.clear();

	// Clear out our old faces
	for (auto p : mesh.parts)
	{
		for (auto f : p->cutFaces)
			delete f;
		p->cutFaces.clear();
		p->isCut = false;
	}

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
			for (int i = 0; auto self : mesh.parts)
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
					if (glm::dot(cutNorm, pc.norm) == -1)
					{
						// Are all of our points in the other cutter?
						bool engulfed = true;
						for (auto v : self->verts)
						{
							bool isOut = true;
							// Full faces for collision testing
							for (auto t : cutP->fullFaces)
							{
								if (t->verts.size() != 3)
									continue;
								// This should get turned into a bulk tester...
								if (testPointInTriEdges(*v->vert + mesh.origin, *t->verts[0]->vert + cutter->origin, *t->verts[1]->vert + cutter->origin, *t->verts[2]->vert + cutter->origin))
								{
									isOut = false;
									break;
								}
							}

							if (isOut)
							{
								engulfed = false;
								break;
							}
						}

						if (engulfed)
						{
							// Clear the face
							for (auto f : self->cutFaces)
								delete f;
							self->cutFaces.clear();
							self->isCut = true;
							// Not worth doing any more cuts...
							break;
						}
						else
						{
							if (self->cutFaces.size() == 0)
							{
								// Give ourselves a face to crack
								inCutFace_t* clone = cloneFace<inCutFace_t>(self);
								clone->meshPart = self;
								self->cutFaces.push_back(clone);
							}

							faceSnips(mesh, *cutter, self, cutP);

							self->isCut = true;

						}
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

template<typename F = face_t>
F* cloneFace(face_t* f)
{
	F* clone = new F;
	defineFace(*clone, { (void*)&f->verts, &vertVectorAccessor }, f->verts.size());
	return clone;
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
