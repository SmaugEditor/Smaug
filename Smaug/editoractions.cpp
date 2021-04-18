#include "editoractions.h"
#include "debugdraw.h"
#include <utils.h>

CNode* CWallExtrudeAction::CreateExtrusion()
{
	CQuadNode* quad = new CQuadNode();
	quad->m_mesh.origin = m_node->Origin();

	meshPart_t* selectedPart = m_selectInfo.side;

	meshPart_t* newFront = quad->m_mesh.parts[0];
	meshPart_t* newBack = quad->m_mesh.parts[1];

	for (int i = 0; auto v : selectedPart->verts)
		*newBack->verts[i++]->vert = *v->vert + m_moveDelta;

	for (int i = 0; auto v : selectedPart->verts)
		*newFront->verts[newFront->verts.size() - 1 - (i++)]->vert = *v->vert;

	return quad;
}

void CWallExtrudeAction::Preview()
{
	meshPart_t* selectedPart = m_selectInfo.side;
	for(auto v : selectedPart->verts)
		DebugDraw().LineDelta(*v->vert + m_node->m_mesh.origin, m_moveDelta, COLOR_BLUE, 0.75f, 0.01f );

}

void CWallExtrudeAction::Act()
{
	CNode* node = CreateExtrusion();
	GetWorldEditor().RegisterNode(node);
	m_quad = node;
	node->Update();
}

void CWallExtrudeAction::Undo()
{
	if (m_quad.IsValid())
		GetWorldEditor().DeleteNode(m_quad.Node());

	
}

void CWallExtrudeAction::Redo()
{
	SASSERT(!m_quad.Node());

	CNode* node = CreateExtrusion();
	GetWorldEditor().AssignID(node, m_quad.ID());
	node->Update();

}

int CVertDragAction::GetSelectionType()
{
	return ACT_SELECT_VERT;
}
