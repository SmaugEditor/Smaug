#pragma once
#include "mesh.h"

bool pointInConvexMeshPart(meshPart_t* part, glm::vec3 pos);
bool pointInConvexLoop(vertex_t* vert, glm::vec3 pos);
bool pointInConvexLoopNoEdges(vertex_t* vert, glm::vec3 pos);
inline bool pointInConvexLoop(halfEdge_t* he, glm::vec3 pos) { return pointInConvexLoop(he->vert, pos); };
inline bool pointInConvexMeshFace(face_t* face, glm::vec3 pos) { return pointInConvexLoop(face->verts.front(), pos); }