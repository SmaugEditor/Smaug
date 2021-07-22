#pragma once
#include "mesh.h"


face_t* sliceMeshPartFace(meshPart_t& mesh, std::vector<face_t*>& faceVec, face_t* face, vertex_t* start, vertex_t* end);

void triangluateMeshPartFaces(meshPart_t& mesh, std::vector<face_t*>& faceVec);
void triangluateMeshPartConvexFaces(meshPart_t& mesh, std::vector<face_t*>& faceVec);
void convexifyMeshPartFaces(meshPart_t& mesh, std::vector<face_t*>& faceVec);

void optimizeParallelEdges(meshPart_t* part, std::vector<face_t*>& faceVec);