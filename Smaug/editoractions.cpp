#include "editoractions.h"

void CWallExtrudeAction::Preview()
{
}

void CWallExtrudeAction::Act()
{

	printf("SELECTED WALL %f\n", m_selectInfo.wall->bottomPoints[0].x);
	
	CQuadNode* quad = GetWorldEditor().CreateQuad();
	quad->m_origin = m_node->m_origin;

	// If we don't flip vertex 1 and 2 here, the tri gets messed up and wont render.

	// 2 to 1 and 1 to 2
	// 
	// 2-----1
	// |     |
	// 1 --- 2

	nodeSide_t* attachedSide = &quad->m_sides[0];
	nodeSide_t* extrudedSide = &quad->m_sides[2];

	attachedSide->vertex1->origin = m_selectInfo.wall->bottomPoints[1];
	attachedSide->vertex2->origin = m_selectInfo.wall->bottomPoints[0];

	extrudedSide->vertex1->origin = m_selectInfo.wall->bottomPoints[0] + m_moveDelta;
	extrudedSide->vertex2->origin = m_selectInfo.wall->bottomPoints[1] + m_moveDelta;

	CConstraint sideConstraint;
	sideConstraint.SetParent(m_node, m_selectInfo.side);
	sideConstraint.SetChild(quad, attachedSide);
	quad->m_constrainedTo[quad->m_constrainedToCount] = sideConstraint;
	m_node->m_constraining.push_back(&quad->m_constrainedTo[quad->m_constrainedToCount]);
	quad->m_constrainedToCount++;

	quad->Update();


	// HACK HACK
	// For some reason we have to update the parent... Even though we just updated them...
	m_node->Update();

	printf("Created New Node\n");
}

void CWallExtrudeAction::Undo()
{
}

void CWallExtrudeAction::Redo()
{
}

int CVertDragAction::GetSelectionType()
{
	return ACT_SELECT_VERT;
}
