#pragma once

#include "mesh.h"

// Creates a convex polygon with n sides and a normal of (0,1,0)
meshPart_t* partFromPolygon(mesh_t* mesh, int sides, glm::vec3 offset);

// Creates a quad that stems out of a edge.
meshPart_t* pullQuadFromEdge(vertex_t* stem, glm::vec3 offset);

// Creates quads from a list of stems. Useful for making cylinders out of faces.
std::vector<vertex_t*> extrudeEdges(std::vector<vertex_t*>& stems, glm::vec3 offset);

// Caps off a stem list with a mesh part. Useful for closing an extrusion.
meshPart_t* endcapEdges(std::vector<vertex_t*>& stems);

// Inverts the normal of a mesh part
void invertNormal(meshPart_t* part);

// Pushes verts from a face into a vector. Vector is guaranteed to be ordered.
void vertexLoopToVector(face_t* face, std::vector<vertex_t*>& stems);