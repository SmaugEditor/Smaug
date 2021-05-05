#pragma once
#include "mesh.h"


// Deletes and clears out all sliced data
void clearSlicedData(slicedMeshPartData_t* sliced);
// Creates and sets up blank sliced data for a mesh part
void fillSlicedData(meshPart_t* part);

void applyCuts(cuttableMesh_t* mesh, std::vector<mesh_t*>& cutters);