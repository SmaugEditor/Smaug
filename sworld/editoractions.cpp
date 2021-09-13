#include "editoractions.h"
#include <shared.h>

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

void CWallExtrudeAction::Select(std::vector<selectionInfo_t> selectInfo)
{
	CBaseDragAction::Select(selectInfo);
	for (int i = 0; auto info : m_selectInfo)
	{
		if (!info.part.IsValid())
		{
			i++;
			continue;
		}

		CNode* node = CreateExtrusion(info.node.Node(), info.part);
		GetWorldEditor().RegisterNode(node);
		m_quads.push_back({ node, i++ });
		node->MarkDirtyPreview();
	}
}

void CWallExtrudeAction::Cancel()
{
	for (int i = 0; auto info: m_selectInfo)
	{
		if (info.part.IsValid())
		{
			CNodeRef quad = m_quads[i++].first;
			if (quad.IsValid())
			{
				GetWorldEditor().DeleteNode(quad.Node());
				info.node->MarkDirty();
			}
		}
	}
	m_quads.clear();
}

void CWallExtrudeAction::Preview()
{
	for (auto pair : m_quads)
	{
		meshPart_t* newBack = pair.first->m_mesh.parts[1];
		meshPart_t* part = m_selectInfo[pair.second].part;
		
		for (int i = 0; auto v : part->verts)
			*newBack->verts[i++]->vert = *v->vert + m_moveDelta;

		pair.first->MarkDirtyPreview();
	}
	

}

bool CWallExtrudeAction::Act()
{
	if (m_quads.size() == 0)
		return false;

	for (auto pair : m_quads)
	{
		meshPart_t* newBack = pair.first->m_mesh.parts[1];
		meshPart_t* part = m_selectInfo[pair.second].part;

		for (int i = 0; auto v : part->verts)
			*newBack->verts[i++]->vert = *v->vert + m_moveDelta;

		pair.first->MarkDirty();
	}

	return true;
}

void CWallExtrudeAction::Undo()
{
	for (int i = 0; auto info: m_selectInfo)
	{
		if (!info.part.IsValid())
			continue;

		CNodeRef quad = m_quads[i++].first;
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
		if (!info.part.IsValid())
			continue;

		CNodeRef quadRef = m_quads[i++].first;
		CNode* node = CreateExtrusion(info.node.Node(), info.part);
		GetWorldEditor().AssignID(node, quadRef.ID());
		node->MarkDirty();
	}
}
