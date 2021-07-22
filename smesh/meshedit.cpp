#include "meshedit.h"


meshPart_t* partFromPolygon(mesh_t* mesh, int sides, glm::vec3 offset)
{
	std::vector<glm::vec3> points;
	for (int i = 0; i < sides; i++)
		points.push_back(glm::vec3(-cos(i * (PI * 2.0f) / sides), 0, sin(i * (PI * 2.0f) / sides)) + offset);

	auto p = addMeshVerts(*mesh, points.data(), points.size());

	return addMeshFace(*mesh, p, sides);
}

meshPart_t* pullQuadFromEdge(vertex_t* stem, glm::vec3 offset)
{
	mesh_t& parent = *parentMesh(stem->edge->face);
	glm::vec3 verts[2] = { *stem->vert + offset, *stem->edge->vert->vert + offset };
	auto p = addMeshVerts(parent, verts, 2);
	glm::vec3* v[4] = { p[0], p[1], stem->edge->vert->vert, stem->vert };
	return addMeshFace(parent, v, 4);
}

std::vector<vertex_t*> extrudeEdges(std::vector<vertex_t*>& stems, glm::vec3 offset)
{
	if (stems.size() < 2)
		return {};

	mesh_t& parent = *parentMesh(stems.front()->edge->face);

	glm::vec3* verts = new glm::vec3[stems.size()];
	for (int i = 0; auto s : stems)
		verts[i++] = *s->vert + offset;
	auto p = addMeshVerts(parent, verts, stems.size());

	std::vector<vertex_t*> newStems;
	for (int i = 0; auto s : stems)
	{
		glm::vec3* v[4] = { p[i], p[(i + 1) % stems.size()], s->edge->vert->vert, s->vert };
		newStems.push_back(addMeshFace(parent, v, 4)->verts[0]);
		i++;
	}
	return newStems;
}


meshPart_t* endcapEdges(std::vector<vertex_t*>& stems)
{
	if (stems.size() < 2)
		return {};

	mesh_t& parent = *parentMesh(stems.front()->edge->face);

	std::vector<glm::vec3*> points;
	for (int i = stems.size() - 1; i >= 0; i--)
		points.push_back(stems[i]->vert);

	return addMeshFace(parent, points.data(), points.size());
}

void invertPartNormal(meshPart_t* part)
{
	if (part->verts.size() < 2 || part->edges.size() < 2)
		return;

	// So that we maintain pairs and etc, we actually have to relink this mesh, not just remake it but backwards

	vertex_t* vs = part->verts.front(), * last = vs, * v;
	halfEdge_t* lastEdge = last->edge, * firstEdge = lastEdge;
	
	
	// Shift everything forward
	last = last->edge->vert;
	vs = last;
	v = last->edge->vert;

	do
	{
		halfEdge_t* ce = last->edge;
		ce->vert = last;
		ce->next = lastEdge;
		last->edge = lastEdge;
		lastEdge = ce;

		last = v;
		v = v->edge->vert;
	} while (last != vs);

}


void vertexLoopToVector(face_t* face, std::vector<vertex_t*>& stems)
{
	vertex_t* vs = face->verts.front(), * v = vs;
	do
	{
		stems.push_back(v);
		v = v->edge->vert;
	} while (v != vs);
}
