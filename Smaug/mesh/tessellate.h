#pragma once
#include "mesh.h"

face_t* sliceMeshPartFace(meshPart_t& mesh, std::vector<face_t*>& faceVec, face_t* face, vertex_t* start, vertex_t* end);
void triangluateMeshPartFaces(meshPart_t& mesh, std::vector<face_t*>& faceVec);