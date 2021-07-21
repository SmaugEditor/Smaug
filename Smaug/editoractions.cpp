#include "editoractions.h"
#include "debugdraw.h"
#include <utils.h>

CNode* CWallExtrudeAction::CreateExtrusion(CNode* node, meshPart_t* part)
{
	CQuadNode* quad = new CQuadNode();
	quad->m_mesh.origin = node->Origin();

	meshPart_t* newFront = quad->m_mesh.parts[0];
	meshPart_t* newBack = quad->m_mesh.parts[1];

	for (int i = 0; auto v : part->verts)
		*newBack->verts[i++]->vert = *v->vert + m_moveDelta;

	for (int i = 0; auto v : part->verts)
		*newFront->verts[newFront->verts.size() - 1 - (i++)]->vert = *v->vert;

	return quad;
}

void CWallExtrudeAction::Preview()
{
	for (auto info : m_selectInfo)
	{
		meshPart_t* selectedPart = info.part;
		glm::vec3 color = glm::dot(selectedPart->normal, m_moveDelta) < 0 ? COLOR_BLUE : COLOR_RED;
		for(auto v : selectedPart->verts)
			DebugDraw().LineDelta(*v->vert + info.node->m_mesh.origin, m_moveDelta, color, 0.75f, 0.01f );
	}

}

void CWallExtrudeAction::Act()
{
	for (auto info : m_selectInfo)
	{
		CNode* node = CreateExtrusion(info.node.Node(), info.part);
		GetWorldEditor().RegisterNode(node);
		m_quads.push_back(node);
		node->MarkDirty();
	}
}

void CWallExtrudeAction::Undo()
{
	for (int i = 0; auto info: m_selectInfo)
	{
		CNodeRef quad = m_quads[i++];
		if (quad.IsValid())
		{
			GetWorldEditor().DeleteNode(quad.Node());
			info.node->MarkDirty();
		}
	}
}

void CWallExtrudeAction::Redo()
{
	for (int i = 0; auto info: m_selectInfo)
	{
		CNodeRef quadRef = m_quads[i++];
		CNode* node = CreateExtrusion(info.node.Node(), info.part);
		GetWorldEditor().AssignID(node, quadRef.ID());
		m_quads.push_back(node);
		node->MarkDirty();
	}
}
